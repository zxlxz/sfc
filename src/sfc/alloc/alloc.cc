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

  const auto p = sys::alloc(layout);
  sfc::expect(p != nullptr, "Global::alloc(size={}, align={}): failed", layout.size, layout.align);
  return p;
}

void Global::dealloc(void* ptr, Layout layout) noexcept {
  if (ptr == nullptr) {
    return;
  }

  sys::dealloc(ptr, layout);
}

auto Global::realloc(void* ptr, Layout layout, usize new_size) noexcept -> void* {
  if (new_size == 0) {
    sys::dealloc(ptr, layout);
    return nullptr;
  }

  const auto new_layout = Layout{.size = new_size, .align = layout.align};
  const auto p = ptr ? sys::realloc(ptr, layout, new_size) : sys::alloc(new_layout);
  sfc::expect(p != nullptr,
              "Global::realloc(size={}, align={}, new_size={}): failed",
              layout.size,
              layout.align,
              new_size);
  return p;
}

}  // namespace sfc::alloc
