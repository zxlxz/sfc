#include <stddef.h>
#include <stdlib.h>

#include "sfc/sys/posix/alloc.h"

namespace sfc::sys::posix {

auto alloc(mem::Layout layout) noexcept -> void* {
  if (layout.size == 0) {
    return nullptr;
  }

  if (layout.align <= alignof(max_align_t)) {
    return ::malloc(layout.size);
  }

  const auto aligned_size = num::align_up(layout.size, layout.align);
  return ::aligned_alloc(layout.align, aligned_size);
}

void dealloc(void* ptr, [[maybe_unused]] mem::Layout layout) noexcept {
  if (ptr == nullptr) {
    return;
  }

  ::free(ptr);
}

auto realloc(void* ptr, mem::Layout layout, usize new_size) noexcept -> void* {
  if (layout.size == new_size) {
    return ptr;
  }

  if (!num::is_power_of_two(layout.align)) {
    return ptr;
  }

  if (layout.align <= alignof(max_align_t)) {
    return ::realloc(ptr, new_size);
  }


  const auto copy_size = cmp::min(layout.size, new_size);
  const auto aligned_size = num::align_up(new_size, layout.align);
  const auto new_ptr = ::aligned_alloc(layout.align, aligned_size);
  if (new_ptr && ptr && copy_size != 0) {
    __builtin_memcpy(new_ptr, ptr, copy_size);
  }
  ::free(ptr);

  return new_ptr;
}

}  // namespace sfc::sys::posix
