#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/alloc.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/alloc.inl"
#endif

#include "sfc/alloc/alloc.h"

namespace sfc::alloc {

auto Global::alloc(Layout layout) noexcept -> void* {
  if (layout.size == 0) {
    return nullptr;
  }
  return sys::alloc(layout);
}

void Global::dealloc(void* ptr, Layout layout) noexcept {
  if (ptr == nullptr) {
    return;
  }
  return sys::dealloc(ptr, layout);
}

auto Global::realloc(void* ptr, Layout layout, usize new_size) noexcept -> void* {
  if (new_size == 0) {
    sys::dealloc(ptr, layout);
    return nullptr;
  }
  return sys::realloc(ptr, layout, new_size);
}

}  // namespace sfc::alloc
