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
  static auto alloc_imp(Layout layout) -> void*;

  static void dealloc_imp(void* ptr, Layout layout);

  static auto realloc_imp(void* ptr, Layout layout, usize new_size) -> void*;

  static auto usable_size(void* ptr) -> usize;

  template <class T>
  auto alloc_one() -> T* {
    const auto layout = Layout::of<T>();
    const auto ptr = this->alloc_imp(layout);
    return static_cast<T*>(ptr);
  }

  template <class T>
  void dealloc_one(T* p) {
    const auto layout = Layout::of<T>();
    this->dealloc_imp(p, layout);
  }

  template <class T>
  auto alloc_array(usize n) -> T* {
    const auto m = Layout::array<T>(n);
    const auto p = this->alloc_imp(m);
    return static_cast<T*>(p);
  }

  template <class T>
  void dealloc_array(T* p, usize n) {
    const auto m = Layout::array<T>(n);
    this->dealloc_imp(p, m);
  }
};

}  // namespace sfc::alloc
