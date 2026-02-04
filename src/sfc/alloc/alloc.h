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
  auto alloc(Layout layout) noexcept -> void* {
    return ::operator new(layout.size);
  }

  void dealloc(void* ptr, [[maybe_unused]] Layout layout) noexcept {
    return ::operator delete(ptr);
  }
};

}  // namespace sfc::alloc
