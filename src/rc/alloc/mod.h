#pragma once

#include "rc/core.h"

namespace rc::alloc {

struct Layout {
  usize _align;
  usize _size;

  template <class T>
  static auto array(usize cnt) -> Layout {
    return {alignof(T), sizeof(T) * cnt};
  }
};

struct Global {
  pub static auto alloc(Layout layout) -> void*;
  pub static auto alloc_zeroed(Layout layout) -> void*;
  pub static auto dealloc(void* p, Layout layout) -> void;
  pub static auto realloc(void* p, Layout layout, usize new_size) -> void*;
};

template <class T>
auto alloc(usize len) -> T* {
  const auto layout = Layout::array<T>(len);
  const auto res = Global::alloc(layout);
  return ptr::cast<T>(res);
}

template <class T>
auto dealloc(T* p, usize cnt) -> void {
  const auto layout = Layout::array<T>(cnt);
  Global::dealloc(p, layout);
}

template <class T>
auto realloc(T* p, usize old_cnt, usize new_cnt) -> T* {
  const auto layout = Layout::array<T>(old_cnt);
  const auto new_size = sizeof(T)*new_cnt;
  const auto res = Global::realloc(p, layout, new_size);
  return ptr::cast<T>(res);
}

}  // namespace rc::alloc
