#pragma once

#include "sfc/core.h"

namespace sfc::alloc {

using mem::Layout;

struct Allocator {
  // require: allocate
  // auto allocate(this auto& self, Layout layout) -> void*;

  // require: deallocate
  // void deallocate(this auto& self, void* ptr, Layout layout);
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
