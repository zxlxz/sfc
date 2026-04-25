#pragma once

#include "sfc/core.h"

namespace sfc::alloc {

using mem::Layout;

struct Global {
  auto alloc(Layout layout) noexcept -> void*;
  void dealloc(void* ptr, Layout layout) noexcept;
  auto realloc(void* ptr, Layout layout, usize new_size) noexcept -> void*;

  auto grow(void* ptr, Layout old_layout, Layout new_layout) noexcept -> void*;
  auto shrink(void* ptr, Layout old_layout, Layout new_layout) noexcept -> void*;
};

}  // namespace sfc::alloc
