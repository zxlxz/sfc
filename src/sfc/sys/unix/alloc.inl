#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::unix {

auto alloc(mem::Layout layout) -> void* {
  const auto size = layout.size;
  const auto align = layout.align;

  if (size == 0) {
    return nullptr;
  }

  if (align <= alignof(max_align_t)) {
    return ::malloc(size);
  }

  return ::aligned_alloc(align, size);
}

void dealloc(void* ptr, [[maybe_unused]] mem::Layout layout) noexcept {
  if (ptr == nullptr) {
    return;
  }

  ::free(ptr);
}

auto realloc(void* ptr, mem::Layout layout, usize new_size) -> void* {
  const auto align = layout.align;
  const auto old_size = layout.size;

  // if old_size == new_size: return old ptr
  if (old_size == new_size) {
    return ptr;
  }

  // if align <= max_align: use realloc
  if (align <= alignof(max_align_t)) {
    return ::realloc(ptr, new_size);
  }

  const auto new_ptr = ::aligned_alloc(align, new_size);
  const auto copy_size = old_size < new_size ? old_size : new_size;
  if (new_ptr && ptr && copy_size != 0) {
    ::memcpy(new_ptr, ptr, copy_size);
  }
  ::free(ptr);

  return new_ptr;
}

}  // namespace sfc::sys::unix
