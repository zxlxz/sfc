#include "alloc.h"

#include "sfc/sys/alloc.h"

namespace sfc::alloc {

namespace sys_imp = sys::alloc;

auto Global::alloc(Layout layout) -> void* {
  if (layout.size == 0) {
    return nullptr;
  }

  return sys_imp::malloc(layout.size);
}

void Global::dealloc(void* ptr, Layout layout) {
  (void)layout;
  if (ptr == nullptr) {
    return;
  }

  sys_imp::free(ptr);
}

auto Global::realloc(void* old_ptr, Layout layout, usize new_size) -> void* {
  if (old_ptr == nullptr) {
    return sys_imp::malloc(new_size);
  }

  if (layout.size == new_size) {
    return old_ptr;
  }

  return sys_imp::realloc(old_ptr, new_size);
}

auto Global::usable_size(void* ptr) -> usize {
  if (ptr == nullptr) {
    return 0;
  }

  return sys_imp::msize(ptr);
}

}  // namespace sfc::alloc
