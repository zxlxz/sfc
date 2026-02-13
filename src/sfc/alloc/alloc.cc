#include "sfc/alloc/alloc.h"

namespace sfc::alloc {

auto Global::alloc(Layout layout) noexcept -> void* {
  if (layout.size == 0) {
    return nullptr;
  }
  return __builtin_malloc(layout.size);
}

void Global::dealloc(void* ptr, [[maybe_unused]] Layout layout) noexcept {
  if (ptr == nullptr) {
    return;
  }
  return __builtin_free(ptr);
}

auto Global::realloc(void* ptr, [[maybe_unused]] Layout layout, usize new_size) noexcept -> void* {
  return __builtin_realloc(ptr, new_size);
}

}  // namespace sfc::alloc
