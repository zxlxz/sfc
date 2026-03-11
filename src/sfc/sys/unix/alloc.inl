#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::unix {

static inline auto alloc(alloc::Layout layout) -> void* {
  if (layout.align <= sizeof(void*)) {
    return ::malloc(layout.size);
  } else {
    return ::aligned_alloc(layout.align, layout.size);
  }
}

static inline auto dealloc(void* ptr, alloc::Layout layout) noexcept -> void {
  (void)layout;
  ::free(ptr);
}

static inline auto realloc(void* ptr, alloc::Layout layout, size_t new_size) -> void* {
  (void)layout;
  return ::realloc(ptr, new_size);
}

}  // namespace sfc::sys
