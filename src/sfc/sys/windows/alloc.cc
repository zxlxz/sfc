#include <malloc.h>

#include "sfc/sys/windows/alloc.h"

namespace sfc::sys::windows {

using max_align_t = double;  // see <cstddef>

auto alloc(mem::Layout layout) noexcept -> void* {
  if (layout.size == 0) {
    return nullptr;
  }

  if (layout.align <= alignof(max_align_t)) {
    return ::malloc(layout.size);
  }
  const auto aligned_size = num::align_up(layout.size, layout.align);
  return ::_aligned_malloc(aligned_size, layout.align);
}

void dealloc(void* ptr, mem::Layout layout) noexcept {
  if (ptr == nullptr) {
    return;
  }

  if (layout.align <= alignof(max_align_t)) {
    ::free(ptr);
    return;
  }

  ::_aligned_free(ptr);
}

auto realloc(void* ptr, mem::Layout layout, usize new_size) noexcept -> void* {
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
