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
  auto realloc(this auto& self, void* ptr, Layout layout, usize new_size) noexcept -> void* {
    const auto new_layout = Layout{new_size, layout.align};
    const auto new_ptr = self.alloc(new_layout);
    if (ptr != nullptr) {
      __builtin_memcpy(new_ptr, ptr, cmp::min(layout.size, new_size));
      self.dealloc(ptr, layout);
    }
    return new_ptr;
  }
};

struct Global : GlobalAlloc {
  auto alloc(Layout layout) noexcept -> void* {
    return ::operator new(layout.size);
  }

  void dealloc(void* ptr, [[maybe_unused]] Layout layout) noexcept {
    return ::operator delete(ptr);
  }
};

}  // namespace sfc::alloc
