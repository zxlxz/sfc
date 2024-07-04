#pragma once

#include "sfc/core.h"

namespace sfc::alloc {

struct Layout {
  usize align;
  usize size;

  template <class T>
  [[sfc_inline]] static auto of() -> Layout {
    return {alignof(T), sizeof(T)};
  }

  template <class T>
  [[sfc_inline]] static auto array(usize len) -> Layout {
    return {alignof(T), len * sizeof(T)};
  }

  template <class T>
  [[sfc_inline]] static auto for_value(const T&) -> Layout {
    return {alignof(T), sizeof(T)};
  }
};

struct Global {
  template <class T>
  auto alloc_one() -> T* {
    const auto layout = Layout::of<T>();
    const auto ptr = this->_alloc(layout);
    return static_cast<T*>(ptr);
  }

  template <class T>
  void dealloc_one(T* p) {
    const auto layout = Layout::of<T>();
    this->_dealloc(p, layout);
  }

  template <class T>
  auto alloc(usize n) -> T* {
    const auto m = Layout::array<T>(n);
    const auto p = this->_alloc(m);
    return static_cast<T*>(p);
  }

  template <class T>
  void dealloc(T* p, usize n) {
    const auto m = Layout::array<T>(n);
    this->_dealloc(p, m);
  }

  template <class T>
  auto realloc(T* old_ptr, usize old_len, usize new_len) -> T* {
    const auto max_len = this->_usable_size(old_ptr) / sizeof(T);
    if (new_len <= max_len) {
      const auto layout = Layout::array<T>(old_len);
      const auto new_ptr = _realloc(old_ptr, layout, new_len * sizeof(T));
      return static_cast<T*>(new_ptr);
    }

    const auto new_ptr = alloc<T>(new_len);
    ptr::uninit_move(old_ptr, new_ptr, old_len);
    ptr::drop_in_place(old_ptr, old_len);
    return new_ptr;
  }

  template <trait::Copy T>
  auto realloc(T* old_ptr, usize old_len, usize new_len) -> T* {
    const auto m = Layout::array<T>(old_len);
    const auto p = this->_realloc(old_ptr, m, new_len * sizeof(T));
    return static_cast<T*>(p);
  }

 private:
  auto _alloc(Layout layout) -> void*;
  void _dealloc(void* ptr, Layout layout);
  auto _realloc(void* ptr, Layout layout, usize new_size) -> void*;
  auto _usable_size(void* ptr) -> usize;
};

}  // namespace sfc::alloc
