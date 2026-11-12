#pragma once

#include "sfc/core.h"

namespace sfc::alloc {

using mem::Layout;

struct Allocator {};

struct System {
  auto allocate(Layout layout) -> void*;
  auto allocate_zeroed(Layout layout) -> void*;
  void deallocate(void* ptr, Layout layout);

  auto grow(void* ptr, Layout old_layout, Layout new_layout) -> void*;
  auto shrink(void* ptr, Layout old_layout, Layout new_layout) -> void*;
};

using Global = System;

auto next_power_of_two(usize val) -> usize;

}  // namespace sfc::alloc
