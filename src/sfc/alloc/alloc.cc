#include "alloc.h"

#include <malloc.h>

namespace sfc::alloc {

auto Global::alloc_imp(Layout layout) -> void* {
  if (layout.size == 0) {
    return nullptr;
  }

  const auto ptr = ::malloc(layout.size);
  return ptr;
}

void Global::dealloc_imp(void* ptr, Layout layout) {
  (void)layout;
  if (ptr == 0) {
    return;
  }

  ::free(ptr);
}

auto Global::realloc_imp(void* old_ptr, Layout layout, usize new_size) -> void* {
  if (layout.size == new_size) {
    return old_ptr;
  }

  if (new_size == 0) {
    this->dealloc_imp(old_ptr, layout);
    return nullptr;
  }

  if (old_ptr == nullptr) {
    const auto new_layout = Layout{.size = new_size, .align = layout.align};
    const auto new_ptr = this->alloc_imp(new_layout);
    return new_ptr;
  }

  const auto ptr = ::realloc(old_ptr, new_size);
  return ptr;
}

auto Global::usable_size(void* p) -> usize {
  if (p == nullptr) {
    return 0;
  }
  const auto size = ::malloc_usable_size(p);
  return size;
}

}  // namespace sfc::alloc
