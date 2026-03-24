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

  const auto p = sys::malloc(layout.size);
  sfc::expect(p, "Global::alloc(size={}, align={}): failed", layout.size, layout.align);
  return p;
}

void Global::dealloc(void* ptr, [[maybe_unused]] Layout layout) noexcept {
  if (ptr == nullptr) {
    return;
  }

  sys::free(ptr);
}

auto Global::realloc(void* ptr, Layout layout, usize new_size) noexcept -> void* {
  if (new_size == 0) {
    if (ptr != nullptr) {
      sys::free(ptr);
    }
    return nullptr;
  }

  if (ptr == nullptr) {
    const auto p = sys::malloc(new_size);
    sfc::expect(p, "Global::realloc(size={}, new_size={}): failed", layout.size, new_size);
    return p;
  }

  const auto p = sys::realloc(ptr, new_size);
  sfc::expect(p, "Global::realloc(size={}, new_size={}): failed", layout.size, new_size);
  return p;
}

}  // namespace sfc::alloc
