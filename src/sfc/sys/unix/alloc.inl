#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::unix {

auto alloc(mem::Layout layout) -> void* {
  if (layout.size == 0) {
    return nullptr;
  }

  if (layout.align <= alignof(max_align_t)) {
    return ::malloc(layout.size);
  }

  return ::aligned_alloc(layout.align, layout.size);
}

void dealloc(void* ptr, [[maybe_unused]] mem::Layout layout) noexcept {
  if (ptr == nullptr) {
    return;
  }
  ::free(ptr);
}

auto realloc(void* ptr, mem::Layout layout, usize new_size) -> void* {
  // if (alignment <= system alignment): use realloc directly
  if (layout.align <= alignof(max_align_t)) {
    return ::realloc(ptr, new_size);
  }

  // if new_size <= old_size:
  //    just return the same block, no need to realloc
  if (new_size <= layout.size) {
    return ptr;
  }

  // (new_size > old_size):
  //    need to alloc new block and copy
  const auto new_ptr = ::aligned_alloc(layout.align, new_size);
  if (new_ptr) ::memcpy(new_ptr, ptr, layout.size);
  ::free(ptr);
  return new_ptr;
}

}  // namespace sfc::sys::unix
