#pragma once

#include "sfc/core.h"

namespace sfc::alloc {

struct Layout {
  usize size;
  usize align;

  template <class T>
  static auto of() -> Layout {
    return {sizeof(T), alignof(T)};
  }

  template <class T>
  static auto array(usize len) -> Layout {
    return {len * sizeof(T), alignof(T)};
  }

  template <class T>
  static auto for_value(const T&) -> Layout {
    return {sizeof(T), alignof(T)};
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

  template <class T>
  auto realloc_array(T* old_ptr, usize old_len, usize new_len) -> T* {
    const auto max_len = this->usable_size(old_ptr) / sizeof(T);
    if (new_len <= max_len || __is_trivially_copyable(T)) {
      const auto layout = Layout::array<T>(old_len);
      const auto new_ptr = this->realloc_imp(old_ptr, layout, new_len * sizeof(T));
      return static_cast<T*>(new_ptr);
    }

    const auto new_ptr = this->alloc_array<T>(new_len);
    ptr::uninit_move(old_ptr, new_ptr, old_len);
    ptr::drop_in_place(old_ptr, old_len);
    return new_ptr;
  }
};

}  // namespace sfc::alloc
