#include "sfc/alloc/mpool.h"
#include "sfc/alloc/list.h"
#include "sfc/sync/mutex.h"

namespace sfc::alloc {

class RawMPool::Bucket {
  struct Block {
    void* ptr;
    usize seq;
  };
  usize _block_size;
  List<Block> _free_list;

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
  auto fast_alloc() -> void* {
    if (_free_list.is_empty()) {
      return nullptr;
    }
    const auto [ptr, _] = _free_list.pop().unwrap();
    return ptr;
  }

  void fast_dealloc(void* ptr, usize seq) {
    _free_list.push(Block{ptr, seq});
  }

  auto block_size() const noexcept -> usize {
    return _block_size;
  }

  auto free_count() const noexcept -> usize {
    return _free_list.len();
  }

  auto free_size() const noexcept -> usize {
    return _free_list.len() * _block_size;
  }

  auto age(usize seq) const noexcept -> f64 {
    if (_free_list.is_empty()) return 0;

    const auto seq0 = _free_list[0].seq;
    if (seq < seq0) return 0;

    const auto age = f64(seq - seq0) * f64(_block_size >> 20U);
    return age;
  }

  auto pop() -> void* {
    if (_free_list.is_empty()) {
      return nullptr;
    }
    const auto [ptr, _] = _free_list.remove(0);
    return ptr;
  }
};

class RawMPool::Inn {
  const usize _max_free_bytes;

  List<Bucket> _buckets{};
  usize _seq{0};
  usize _total_bytes{0};
  usize _free_bytes{0};
  mutable sync::Mutex _mutex{};

 public:
  explicit Inn(usize max_free_bytes) : _max_free_bytes{max_free_bytes} {}
  ~Inn() {}

 public:
  auto total_bytes() const noexcept -> usize {
    return _total_bytes;
  }

  auto free_bytes() const noexcept -> usize {
    return _free_bytes;
  }

  auto fast_alloc(usize size) -> void* {
    auto lock = _mutex.lock();
    auto& bkt = bucket(size);

    auto ptr = bkt.fast_alloc();
    if (ptr != nullptr) {
      _free_bytes -= size;
    }
    return ptr;
  }

  void fast_dealloc(void* ptr, usize size) {
    auto lock = _mutex.lock();
    auto& bkt = bucket(size);
    bkt.fast_dealloc(ptr, _seq++);
    _free_bytes += size;
  }

  void push([[maybe_unused]] void* ptr, usize size) {
    auto lock = _mutex.lock();
    _total_bytes += size;
  }

  auto pop() -> Tuple<void*, usize> {
    auto lock = _mutex.lock();

    // If the free bytes is less than the max free bytes
    // we don't need to pop any block from the pool.
    if (_free_bytes < _max_free_bytes) {
      return {nullptr, 0U};
    }

    auto* bkt = this->find_oldest_bucket();
    if (bkt == nullptr) {
      return {nullptr, 0U};
    }

    const auto blk_ptr = bkt->pop();
    if (blk_ptr == nullptr) {
      return {nullptr, 0U};
    }

    const auto blk_size = bkt->block_size();
    _free_bytes -= blk_size;
    _total_bytes -= blk_size;

    return {blk_ptr, blk_size};
  }

 private:
  auto bucket(usize size) -> Bucket& {
    auto opt = _buckets.iter_mut().find([&](auto& b) { return b.block_size() == size; });
    if (opt.is_some()) {
      return *opt;
    }

    auto& bkt = _buckets.push(Bucket{size});
    return bkt;
  }

  auto find_oldest_bucket() -> Bucket* {
    if (_buckets.is_empty()) {
      return nullptr;
    }

    auto max_age = 0.0;
    Bucket* bkt = nullptr;
    for (auto& b : _buckets) {
      const auto a = b.age(_seq);
      if (a > max_age) {
        max_age = a;
        bkt = &b;
      }
    }

    return bkt;
  }
};

RawMPool::RawMPool() noexcept = default;
RawMPool::~RawMPool() noexcept = default;

RawMPool::RawMPool(RawMPool&& other) noexcept = default;
RawMPool& RawMPool::operator=(RawMPool&& other) noexcept = default;

auto RawMPool::with_capacity(usize cap) noexcept -> RawMPool {
  auto res = RawMPool{};
  res._inn = Box<Inn>::new_(cap);
  return res;
}

auto RawMPool::total_bytes() const noexcept -> usize {
  auto* inn = _inn.as_ptr();
  return inn ? inn->total_bytes() : 0;
}

auto RawMPool::free_bytes() const noexcept -> usize {
  auto* inn = _inn.as_ptr();
  return inn ? inn->free_bytes() : 0;
}

auto RawMPool::inner() -> Inn& {
  const auto kMaxFreeBytes = 1ULL << 30U;  // 1GB
  if (_inn.is_null()) {
    _inn = Box<Inn>::new_(kMaxFreeBytes);
  }
  return *_inn;
}

auto RawMPool::fast_alloc(usize size) -> void* {
  auto& inn = this->inner();
  return inn.fast_alloc(size);
}

void RawMPool::fast_dealloc(void* ptr, usize size) {
  auto& inn = this->inner();
  inn.fast_dealloc(ptr, size);
}

auto RawMPool::push(void* ptr, usize size) -> void {
  auto& inn = this->inner();
  inn.push(ptr, size);
}

auto RawMPool::pop() -> Tuple<void*, usize> {
  auto& inn = this->inner();
  return inn.pop();
}

}  // namespace sfc::alloc
