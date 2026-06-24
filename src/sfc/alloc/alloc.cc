#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/alloc.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/alloc.inl"
#endif

#include "sfc/alloc/alloc.h"

namespace sfc::alloc {

auto Global::alloc(Layout layout) noexcept -> void* {
  sfc::assert_(num::is_power_of_two(layout.align), "Global::alloc: invalid align({})", layout.align);

  if (layout.size == 0) {
    return nullptr;
  }

  // no need to check if ptr is nullptr here
  // the caller should handle it
  const auto ptr = sys::alloc(layout);
  return ptr;
}

void Global::dealloc(void* ptr, Layout layout) noexcept {
  if (ptr == nullptr) {
    return;
  }

  sys::dealloc(ptr, layout);
}

auto Global::realloc(void* ptr, Layout layout, usize new_size) noexcept -> void* {
  sfc::assert_(num::is_power_of_two(layout.align), "Global::realloc: invalid align({})", layout.align);

  if (new_size == layout.size) {
    return ptr;
  }

  return sys::realloc(ptr, layout, new_size);
}

auto Global::grow(void* ptr, Layout old_layout, Layout new_layout) noexcept -> void* {
  sfc::assert_(old_layout.align == new_layout.align, "Global::grow: alignment not match (old({}), new({}))", old_layout.align, new_layout.align);

  if (old_layout.size >= new_layout.size) {
    return ptr;
  }

  if (old_layout.size == 0) {
    return Global::alloc(new_layout);
  }

  return Global::realloc(ptr, old_layout, new_layout.size);
}

auto Global::shrink(void* ptr, Layout old_layout, Layout new_layout) noexcept -> void* {
  sfc::assert_(old_layout.align == new_layout.align, "Global::shrink: alignment not match (old({}), new({}))", old_layout.align, new_layout.align);

  if (old_layout.size <= new_layout.size) {
    return ptr;
  }

  if (new_layout.size == 0) {
    Global::dealloc(ptr, old_layout);
    return nullptr;
  }

  return Global::realloc(ptr, old_layout, new_layout.size);
}

}  // namespace sfc::alloc
