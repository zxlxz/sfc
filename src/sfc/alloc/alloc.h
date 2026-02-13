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

struct GlobalAlloc {
  template <class T>
  auto alloc_array(this auto& self, usize len) -> T* {
    return static_cast<T*>(self.alloc(Layout::array<T>(len)));
  }

  template <class T>
  void dealloc_array(this auto& self, T* ptr, usize len) {
    return self.dealloc(ptr, Layout::array<T>(len));
  }

  template <class T>
  auto realloc_array(this auto& self, T* ptr, usize len, usize new_len, usize used) -> T* {
    if constexpr (trait::tv_copy_<T> || trait::tv_dtor_<T>) {
      return static_cast<T*>(self.realloc(ptr, Layout::array<T>(len), new_len * sizeof(T)));
    } else if (used == 0 || new_len <= len) {
      return static_cast<T*>(self.realloc(ptr, Layout::array<T>(len), new_len * sizeof(T)));
    } else {
      const auto new_ptr = self.template alloc_array<T>(new_len);
      ptr::uninit_move(ptr, new_ptr, used);
      self.dealloc_array(ptr, len);
      return new_ptr;
    }
  }
};

struct Global : GlobalAlloc {
  auto alloc(Layout layout) noexcept -> void*;
  void dealloc(void* ptr, Layout layout) noexcept;
  auto realloc(void* ptr, Layout layout, usize new_size) noexcept -> void*;
};

}  // namespace sfc::alloc
