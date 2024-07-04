#include "alloc.h"

#include <malloc.h>

namespace sfc::alloc {

auto Global::_alloc(Layout layout) -> void* {
  if (layout.size == 0) {
    return nullptr;
  }

  const auto ptr = ::malloc(layout.size);
  return ptr;
}

void Global::_dealloc(void* ptr, Layout layout) {
  (void)layout;
  if (ptr == 0) {
    return;
  }

  ::free(ptr);
}

auto Global::_realloc(void* old_ptr, Layout layout, usize new_size) -> void* {
  if (layout.size == new_size) {
    return old_ptr;
  }

  if (new_size == 0) {
    this->_dealloc(old_ptr, layout);
    return nullptr;
  }

  if (old_ptr == nullptr) {
    const auto new_layout = Layout{.align = layout.align, .size = new_size};
    const auto new_ptr = this->_alloc(new_layout);
    return new_ptr;
  }

  const auto ptr = ::realloc(old_ptr, new_size);
  return ptr;
}

auto Global::_usable_size(void* p) -> usize {
  if (p == nullptr) {
    return 0;
  }
  const auto size = ::malloc_usable_size(p);
  return size;
}

}  // namespace sfc::alloc
