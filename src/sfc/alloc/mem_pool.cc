#include "sfc/alloc/mem_pool.h"
#include "sfc/alloc/list.h"
#include "sfc/sync/mutex.h"

namespace sfc::mem_pool {

static constexpr auto kDefaultAlign = alignof(double);

class Bucket {
  struct Node {
    void* ptr;
    usize seq;
  };

  usize _block_size;
  List<Node> _free_list;

 public:
  explicit Bucket(usize block_size) noexcept : _block_size{block_size} {}

  ~Bucket() noexcept {}

  Bucket(Bucket&& other) noexcept
      : _block_size{mem::take(other._block_size)}, _free_list{mem::move(other._free_list)} {}

  Bucket& operator=(Bucket&& other) noexcept {
    if (this != &other) {
      _block_size = mem::take(other._block_size);
      _free_list = mem::move(other._free_list);
    }
    return *this;
  }

 public:
  auto block_size() const -> usize {
    return _block_size;
  }

  auto fast_alloc() -> void* {
    if (_free_list.is_empty()) {
      return nullptr;
    }
    const auto [ptr, _] = _free_list.pop().unwrap();
    return ptr;
  }

  void fast_dealloc(void* ptr, usize seq) {
    _free_list.push({ptr, seq});
  }

  auto age(usize seq) const noexcept -> f64 {
    if (_free_list.is_empty()) {
      return 0;
    }

    const auto seq0 = _free_list[0].seq;
    if (seq < seq0) {
      return 0;
    }

    const auto age = f64(seq - seq0) * f64(_block_size >> 20U);
    return age;
  }

  struct Block {
    void* ptr;
    usize size;
  };

  auto pop_first() -> Block {
    if (_free_list.is_empty()) {
      return {nullptr, 0};
    }
    const auto [ptr, _] = _free_list.remove(0);
    return {ptr, _block_size};
  }
};

Pool::Pool(usize cap) noexcept : _cap{cap}, _buckets{} {}

Pool::~Pool() noexcept {}

auto Pool::total_bytes() const noexcept -> usize {
  return _total_bytes;
}

auto Pool::free_bytes() const noexcept -> usize {
  return _free_bytes;
}

auto Pool::bucket(usize size) -> Bucket& {
  auto opt = _buckets.iter_mut().find([&](auto& b) { return b.block_size() == size; });
  if (opt.is_some()) {
    return *opt;
  }

  auto& bkt = _buckets.push(Bucket{size});
  return bkt;
}

auto Pool::find_oldest_bucket() -> Bucket& {
  Bucket* bkt = &_buckets[0];

  auto max_age = 0.0;
  for (auto& b : _buckets) {
    const auto a = b.age(_seq);
    if (a > max_age) {
      max_age = a;
      bkt = &b;
    }
  }

  return *bkt;
}

auto Pool::fast_alloc(usize size) -> void* {
  auto lock = _mutex.lock();
  auto& bkt = bucket(size);

  auto ptr = bkt.fast_alloc();
  if (ptr != nullptr) {
    _free_bytes -= size;
  }
  return ptr;
}

void Pool::fast_dealloc(void* ptr, usize size) {
  auto lock = _mutex.lock();
  auto& bkt = bucket(size);
  bkt.fast_dealloc(ptr, _seq++);
  _free_bytes += size;
}

auto Pool::alloc(usize size) -> void* {
  if (auto ptr = this->fast_alloc(size)) {
    return ptr;
  }

  this->recycling(false, size);
  auto ptr = this->slow_alloc({size, kDefaultAlign});

  // register
  if (ptr) {
    _total_bytes += size;
  }

  return ptr;
}

void Pool::dealloc(void* ptr, usize size) {
  this->fast_dealloc(ptr, size);
}

auto Pool::recycling(bool force, usize cap) -> usize {
  if (_buckets.is_empty()) {
    return 0U;
  }

  auto amt = usize{0};
  while (force || amt < cap) {
    if (!force && _free_bytes < _cap) {
      break;
    }

    auto& bucket = this->find_oldest_bucket();
    const auto block = bucket.pop_first();
    if (block.ptr == nullptr) {
      break;
    }

    this->slow_dealloc(block.ptr, {block.size, kDefaultAlign});
    _free_bytes -= block.size;
    _total_bytes -= block.size;
    amt += block.size;
  }
  return amt;
}

auto Pool::global() -> Pool& {
  static auto pool = XPool{alloc::Global{}};
  return pool;
}

Allocator::Allocator(Pool& pool) : _pool{&pool} {}

Allocator::~Allocator() {}

auto Allocator::allocate(Layout layout) -> void* {
  return _pool->alloc(layout.size);
}

void Allocator::deallocate(void* ptr, Layout layout) {
  _pool->dealloc(ptr, layout.size);
}

}  // namespace sfc::mem_pool
