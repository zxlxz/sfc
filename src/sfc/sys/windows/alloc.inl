#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::windows {

using mem::Layout;

static inline auto alloc(Layout layout) -> void* {
  if (layout.size == 0) {
    return nullptr;
  }
  if (layout.align <= alignof(max_align_t)) {
    return ::malloc(layout.size);
  }
  return ::_aligned_malloc(layout.size, layout.align);
}


static inline void dealloc(void* ptr, Layout layout) noexcept {
  if (ptr == nullptr) {
    return;
  }
  if (layout.align <= alignof(max_align_t)) {
    ::free(ptr);
  } else {
    ::_aligned_free(ptr);
  }
}

static inline auto realloc(void* ptr, Layout layout, usize new_size) -> void* {
  if (layout.align <= alignof(max_align_t)) {
    return ::realloc(ptr, new_size);
  }

  return ::_aligned_realloc(ptr, new_size, layout.align);
}

}  // namespace sfc::sys::windows
