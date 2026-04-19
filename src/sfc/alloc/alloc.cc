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

  const auto p = sys::alloc(layout);
  sfc::expect(p, "Global::alloc(size={}, align={}): failed", layout.size, layout.align);
  return p;
}

void Global::dealloc(void* ptr, [[maybe_unused]] Layout layout) noexcept {
  if (ptr == nullptr) {
    return;
  }
  sys::dealloc(ptr, layout);
}

auto Global::realloc(void* ptr, Layout layout, usize new_size) noexcept -> void* {
  // since the old ptr will always be valid
  // we can just return it when the new layout is invalid
  if (layout.align == 0 || (layout.align & (layout.align - 1)) != 0) {
    return ptr;
  }
  if (layout.size == 0 || layout.size % layout.align != 0) {
    return ptr;
  }
  if (new_size == layout.size || new_size % layout.align != 0) {
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

}  // namespace sfc::alloc
