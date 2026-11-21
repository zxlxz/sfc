#pragma once

#include "sfc/core.h"

namespace sfc::alloc {

using mem::Layout;

struct System {
  void* allocate(Layout layout);
  void* allocate_zeroed(Layout layout);
  void deallocate(void* ptr, Layout layout);

  void* grow(void* ptr, Layout layout, usize new_size);
  void* shrink(void* ptr, Layout layout, usize new_size);
};

struct FixedBuf {
  Slice<u8> _buf;
  usize _end_idx{0};

 public:
  void* allocate(Layout layout);
  void* allocate_zeroed(Layout layout);
  void deallocate(void* ptr, Layout layout);

  void* grow(void* ptr, Layout layout, usize new_size);
  void* shrink(void* ptr, Layout layout, usize new_size);

  bool is_last_allocation(void* ptr, Layout layout) const;
};

template <class A>
class Arena {
  [[maybe_unused]] A _allocator;

 public:
  explicit Arena(A allocator = {}) : _allocator{mem::move(allocator)} {}

  void* allocate(Layout layout);
  void* allocate_zeroed(Layout layout);
  void deallocate(void* ptr, Layout layout);

  void* grow(void* ptr, Layout layout, usize new_size);
  void* shrink(void* ptr, Layout layout, usize new_size);
};

using Global = System;

}  // namespace sfc::alloc
