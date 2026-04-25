#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/alloc.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/alloc.inl"
#endif

#include "sfc/alloc/alloc.h"

namespace sfc::alloc {

auto Global::alloc(Layout layout) noexcept -> void* {
  if (layout.align == 0 || (layout.align & (layout.align - 1)) != 0) {
    return nullptr;
  }

  if (layout.size == 0 || layout.size % layout.align != 0) {
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
  if (layout.align == 0 || (layout.align & (layout.align - 1)) != 0) {
    return ptr;
  }

  if (layout.size % layout.align != 0 || new_size % layout.align != 0) {
    return ptr;
  }

  if (new_size == layout.size) {
    return ptr;
  }

  if (new_size == 0) {
    sys::dealloc(ptr, layout);
    return nullptr;
  }

  if (ptr == nullptr) {
    return sys::alloc({new_size, layout.align});
  }

  return sys::realloc(ptr, layout, new_size);
}

auto Global::grow(void* ptr, Layout old_layout, Layout new_layout) noexcept -> void* {
  if (old_layout.align != new_layout.align) {
    panic::expect(false,
                  "alloc::Global::grow: alignment cannot be changed (old={}, new={})",
                  old_layout.align,
                  new_layout.align);
  }

  if (old_layout.size >= new_layout.size) {
    return ptr;
  }

  if (old_layout.size == 0) {
    return this->alloc(new_layout);
  }

  return sys::realloc(ptr, old_layout, new_layout.size);
}

auto Global::shrink(void* ptr, Layout old_layout, Layout new_layout) noexcept -> void* {
  if (old_layout.align != new_layout.align) {
    panic::expect(false,
                  "alloc::Global::shrink: alignment cannot be changed (old={}, new={})",
                  old_layout.align,
                  new_layout.align);
  }

  if (old_layout.size <= new_layout.size) {
    return ptr;
  }

  if (new_layout.size == 0) {
    this->dealloc(ptr, old_layout);
    return nullptr;
  }

  return this->realloc(ptr, old_layout, new_layout.size);
}

}  // namespace sfc::alloc
