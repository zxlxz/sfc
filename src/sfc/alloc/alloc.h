#pragma once

#include "sfc/core.h"

namespace sfc::alloc {

struct Layout {
  usize size;
  usize align;

 public:
  template <class T>
  static auto of() -> Layout {
    return Layout{.size = sizeof(T), .align = alignof(T)};
  }

  template <class T>
  static auto array(usize len) -> Layout {
    return Layout{.size = len * sizeof(T), .align = alignof(T)};
  }

  template <class T>
  static auto for_value(const T&) -> Layout {
    return Layout{.size = sizeof(T), .align = alignof(T)};
  }
};

struct Global {
  static auto alloc(Layout layout) -> void*;

  static void dealloc(void* ptr, Layout layout);

  static auto realloc(void* ptr, Layout layout, usize new_size) -> void*;

  static auto usable_size(void* ptr) -> usize;
};

}  // namespace sfc::alloc
