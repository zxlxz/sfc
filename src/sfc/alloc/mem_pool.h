#pragma once

#include "sfc/alloc/alloc.h"
#include "sfc/alloc/boxed.h"

namespace sfc::mem_pool {

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
class Pool {
  A _alloc{};
  RawPool _inn{};

 public:
  explicit Pool(A alloc = {}) : _alloc{mem::move(alloc)} {}

  ~Pool() noexcept {
    this->recycling(0, true);
  }

  Pool(Pool&& other) noexcept = default;
  Pool& operator=(Pool&& other) noexcept = default;

  static auto with_allocator(A alloc) noexcept -> Pool {
    return Pool{mem::move(alloc)};
  }

 public:
  auto total_bytes() const noexcept -> usize {
    return _inn.total_bytes();
  }

  auto free_bytes() const noexcept -> usize {
    return _inn.free_bytes();
  }

  auto alloc(usize size) -> void* {
    if (auto ptr = _inn.fast_alloc(size)) {
      return ptr;
    }

    this->recycling(size, false);
    return this->sys_allocate(size);
  }

  void dealloc(void* ptr, usize size) {
    _inn.fast_dealloc(ptr, size);
  }

 private:
  auto sys_allocate(usize size) -> void* {
    constexpr usize kAlignSize = 16U;
    auto ptr = _alloc.allocate(Layout{size, kAlignSize});
    _inn.push({ptr, size});
    return ptr;
  }

  void sys_deallocate(void* ptr, usize size) {
    constexpr usize kAlignSize = 16U;
    _alloc.deallocate(ptr, Layout{size, kAlignSize});
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

template <class A = alloc::Global>
struct Allocator {
  using Pool = mem_pool::Pool<A>;

  static auto pool() noexcept -> Pool& {
    static auto pool = Pool{};
    return pool;
  }

  static void* allocate(Layout layout) {
    auto& pool = Allocator::pool();
    return pool.allocate(layout);
  }

  static void deallocate(void* ptr, Layout layout) {
    auto& pool = Allocator::pool();
    pool.dealloc(ptr, layout);
  }
};

}  // namespace sfc::mem_pool
