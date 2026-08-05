#pragma once

#include "sfc/alloc/alloc.h"
#include "sfc/alloc/boxed.h"

namespace sfc::alloc {

class RawMPool {
  class Inn;
  class Bucket;
  Box<Inn> _inn;

 public:
  RawMPool() noexcept;
  ~RawMPool() noexcept;
  RawMPool(RawMPool&& other) noexcept;
  RawMPool& operator=(RawMPool&& other) noexcept;

  static auto with_capacity(usize cap) noexcept -> RawMPool;

 public:
  auto inner() -> Inn&;
  auto total_bytes() const noexcept -> usize;
  auto free_bytes() const noexcept -> usize;

  auto fast_alloc(usize size) -> void*;
  void fast_dealloc(void* ptr, usize size);

  auto push(void* ptr, usize size) -> void;
  auto pop() -> Tuple<void*, usize>;
};

template <class A>
class MPool {
  static constexpr usize kAlignSize = 16U;

  A _alloc{};
  RawMPool _inn{};

 public:
  explicit MPool(A alloc = {}) : _alloc{mem::move(alloc)} {}

  ~MPool() noexcept {
    // auto dealloc pooled blocks is dangerous
    // user should dealloc pooled blocks manually.
  }

  MPool(MPool&& other) noexcept = default;
  MPool& operator=(MPool&& other) noexcept = default;

  static auto with_allocator(A alloc) noexcept -> MPool {
    return MPool{mem::move(alloc)};
  }

 public:
  auto total_bytes() const noexcept -> usize {
    return _inn.total_bytes();
  }

  auto free_bytes() const noexcept -> usize {
    return _inn.free_bytes();
  }

  auto allocate(usize size) -> void* {
    if (auto ptr = _inn.fast_alloc(size)) {
      return ptr;
    }

    this->sys_deallocate(size);
    auto ptr = _alloc.allocate(Layout{size, kAlignSize});
    _inn.push(ptr, size);
    return ptr;
  }

  void dealloc(void* ptr, Layout layout) {
    _inn.fast_dealloc(ptr, layout.size);
  }

 private:
  auto sys_deallocate(usize cap) -> bool {
    auto amt = usize{0};
    while (amt < cap) {
      const auto [blk_ptr, blk_size] = _inn.pop();
      if (blk_ptr == nullptr) {
        break;
      }
      _alloc.deallocate(blk_ptr, Layout{blk_size, kAlignSize});
      amt += blk_size;
    }
    return amt >= cap;
  }
};

}  // namespace sfc::alloc
