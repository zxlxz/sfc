#include "alloc.h"

#include "sfc/sys/alloc.h"

namespace sfc::alloc {

namespace sys_imp = sys::alloc;

auto Global::alloc_imp(Layout layout) -> void* {
  if (layout.size == 0) {
    return nullptr;
  }

  const auto ptr = sys_imp::malloc(layout.size);
  return ptr;
}

void Global::dealloc_imp(void* ptr, Layout layout) {
  (void)layout;
  if (ptr == 0) {
    return;
  }

  sys_imp::free(ptr);
}

auto Global::realloc_imp(void* old_ptr, Layout layout, usize new_size) -> void* {
  (void)layout;
  return sys_imp::realloc(old_ptr, new_size);
}

auto Global::usable_size(void* p) -> usize {
  if (p == nullptr) {
    return 0;
  }
  const auto size = sys_imp::msize(p);
  return size;
}

}  // namespace sfc::alloc
