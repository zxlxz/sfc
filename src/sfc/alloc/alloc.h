#pragma once

#include "sfc/core.h"

namespace sfc::alloc {

using mem::Layout;

struct System {
  static void* allocate(Layout layout);
  static void deallocate(void* ptr, Layout layout);

  static void* grow(void* ptr, Layout layout, usize new_size);
  static void* shrink(void* ptr, Layout layout, usize new_size);
};

using Global = System;

}  // namespace sfc::alloc
