#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::windows {

using mem::Layout;

static inline auto alloc(Layout layout) -> void* {
  const auto size = layout.size;
  const auto align = layout.align;

  if (size == 0) {
    return nullptr;
  }

  if (align <= alignof(max_align_t)) {
    return ::malloc(size);
  }

  return ::_aligned_malloc(layout.size, layout.align);
}

static inline void dealloc(void* ptr, Layout layout) noexcept {
  const auto align = layout.align;

  if (ptr == nullptr) {
    return;
  }

  if (align <= alignof(max_align_t)) {
    ::free(ptr);
    return;
  }

  ::_aligned_free(ptr);
}

static inline auto realloc(void* ptr, Layout layout, usize new_size) -> void* {
  const auto align = layout.align;

  if (align <= alignof(max_align_t)) {
    return ::realloc(ptr, new_size);
  }

  return ::_aligned_realloc(ptr, new_size, align);
}

}  // namespace sfc::sys::windows
