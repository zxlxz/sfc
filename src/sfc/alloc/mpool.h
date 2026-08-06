#pragma once

#include "sfc/alloc/alloc.h"
#include "sfc/alloc/boxed.h"

namespace sfc::mpool {

using mem::Layout;

class RawPool {
  class Inn;
  Box<Inn> _inn;

 public:
  RawPool() noexcept;
  ~RawPool() noexcept;
  RawPool(RawPool&& other) noexcept;
  RawPool& operator=(RawPool&& other) noexcept;

  static auto with_capacity(usize cap) noexcept -> RawPool;

 public:
  auto inner() -> Inn&;
  auto total_bytes() const noexcept -> usize;
  auto free_bytes() const noexcept -> usize;

  auto fast_alloc(usize size) -> void*;
  void fast_dealloc(void* ptr, usize size);

  struct Block {
    void* addr;
    usize size;
  };
  void push(Block block);
  auto pop(bool force) -> Block;
};

template <class A>
class MPool {
  static constexpr usize kAlignSize = 16U;

  A _alloc{};
  RawPool _inn{};

 public:
  explicit MPool(A alloc = {}) : _alloc{mem::move(alloc)} {}

  ~MPool() noexcept {
    this->recycling(usize{0}, true);
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

    this->recycling(size, false);
    return this->sys_allocate(size);
  }

  void dealloc(void* ptr, mem::Layout layout) {
    _inn.fast_dealloc(ptr, layout.size);
  }

 private:
  auto sys_allocate(usize size) -> void* {
    auto ptr = _alloc.allocate(mem::Layout{size, kAlignSize});
    _inn.push({ptr, size});
    return ptr;
  }

  void sys_deallocate(void* ptr, usize size) {
    _alloc.deallocate(ptr, mem::Layout{size, kAlignSize});
  }

  auto recycling(usize cap, bool force) -> usize {
    auto amt = usize{0};
    while (force || amt < cap) {
      const auto blk = _inn.pop(force);
      if (blk.addr == nullptr) {
        break;
      }
      this->sys_deallocate(blk.addr, blk.size);
      amt += blk.size;
    }
    return amt;
  }
};

}  // namespace sfc::mpool
