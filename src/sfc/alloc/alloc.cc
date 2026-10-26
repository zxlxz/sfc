#include "sfc/alloc/alloc.h"
#include "sfc/sys/alloc.h"

namespace sfc::alloc {

// 1TB
constexpr auto kMaxAllocSize = usize{1024ULL << 30};

void* System::allocate(Layout layout) {
  if (layout.size == 0 || layout.size >= kMaxAllocSize) {
    return nullptr;
  }

  return sys::alloc(layout);
}

void* System::alloc_zeroed(Layout layout) {
  if (layout.size == 0 || layout.size >= kMaxAllocSize) {
    return nullptr;
  }

  return sys::alloc_zeroed(layout);
}

void System::deallocate(void* ptr, Layout layout) {
  if (ptr == nullptr) {
    return;
  }

  sys::dealloc(ptr, layout);
}

void* System::grow(void* ptr, Layout old_layout, Layout new_layout) {
  if (old_layout.size >= new_layout.size) {
    return ptr;
  }

  if (old_layout.size == 0) {
    return this->allocate(new_layout);
  }

  return sys::realloc(ptr, old_layout, new_layout.size);
}

void* System::shrink(void* ptr, Layout old_layout, Layout new_layout) {
  if (old_layout.size <= new_layout.size) {
    return ptr;
  }

  if (new_layout.size == 0) {
    this->deallocate(ptr, old_layout);
    return nullptr;
  }

  return sys::realloc(ptr, old_layout, new_layout.size);
}

}  // namespace sfc::alloc
