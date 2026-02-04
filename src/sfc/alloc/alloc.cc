#include "sfc/alloc/alloc.h"

namespace sfc::alloc {

auto Global::alloc(Layout layout) noexcept -> void* {
  if (layout.size == 0) {
    return nullptr;
  }

  const auto ptr = __builtin_malloc(layout.size);
  return ptr;
}

void Global::dealloc(void* ptr, Layout layout) noexcept {
  (void)layout;
  if (ptr == nullptr) {
    return;
  }

  return __builtin_free(ptr);
}

}  // namespace sfc::alloc
