#pragma once

#include "sfc/core.h"

namespace sfc::alloc {

struct Layout {
  usize size{0};
  usize align{1};

 public:
  template <class T>
  static auto array(usize n) noexcept -> Layout {
    return Layout{n * sizeof(T), alignof(T)};
  }
};

struct Global {
  static auto alloc(Layout layout) noexcept -> void*;
  static void dealloc(void* ptr, Layout layout) noexcept;
};

}  // namespace sfc::alloc
