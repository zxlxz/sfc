#include "sfc/alloc/alloc.h"
#include "sfc/sys/alloc.h"

namespace sfc::alloc {

namespace sys_imp = sys::alloc;

auto Global::alloc(Layout layout) noexcept -> void* {
  if (layout.size == 0) {
    return nullptr;
  }
  return sys_imp::malloc(layout.size);
}

void Global::dealloc(void* ptr, [[maybe_unused]] Layout layout) noexcept {
  if (ptr == nullptr) {
    return;
  }
  return sys_imp::free(ptr);
}

auto Global::realloc(void* ptr, [[maybe_unused]] Layout layout, usize new_size) noexcept -> void* {
  if (new_size == 0) {
    if (ptr != nullptr) {
      sys_imp::free(ptr);
    }
    return nullptr;
  }

  if (ptr == nullptr) {
    return sys_imp::malloc(new_size);
  }

  return sys_imp::realloc(ptr, new_size);
}

}  // namespace sfc::alloc
