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

using Global = System;

}  // namespace sfc::alloc

namespace sfc {
using alloc::Global;
}
