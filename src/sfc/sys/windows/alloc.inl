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

  const auto aligned_size = num::align_up(layout.size, layout.align);
  return ::_aligned_malloc(aligned_size, layout.align);
}

static inline void dealloc(void* ptr, Layout layout) noexcept {
  if (ptr == nullptr) {
    return;
  }

  if (layout.align <= alignof(max_align_t)) {
    ::free(ptr);
    return;
  }

  ::_aligned_free(ptr);
}

static inline auto realloc(void* ptr, Layout layout, usize new_size) -> void* {
  if (layout.size == new_size) {
    return ptr;
  }

  if (layout.align <= alignof(max_align_t)) {
    return ::realloc(ptr, new_size);
  }

  const auto aligned_size = num::align_up(new_size, layout.align);
  return ::_aligned_realloc(ptr, aligned_size, layout.align);
}

}  // namespace sfc::sys::windows
