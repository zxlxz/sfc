#pragma once

#include "sfc/core.h"

namespace sfc::alloc {

using mem::Layout;

struct Allocator {
};

struct System {
  void* allocate(Layout layout);
  void* allocate_zeroed(Layout layout);
  void deallocate(void* ptr, Layout layout);

  void* grow(void* ptr, Layout old_layout, Layout new_layout);
  void* shrink(void* ptr, Layout old_layout, Layout new_layout);
};

using Global = System;

}  // namespace sfc::alloc
