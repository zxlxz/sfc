#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/alloc.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/alloc.inl"
#endif

#include "sfc/alloc/alloc.h"

namespace sfc::alloc {

void* System::allocate(Layout layout) {
  if (layout.size == 0) {
    return nullptr;
  }

  return sys::alloc(layout);
}

void System::deallocate(void* ptr, Layout layout) {
  if (ptr == nullptr) {
    return;
  }

  sys::dealloc(ptr, layout);
}

void* System::grow(void* ptr, Layout layout, usize new_size) {
  if (layout.size >= new_size) {
    return ptr;
  }

  if (layout.size == 0) {
    return sys::alloc(Layout{new_size, layout.align});
  }

  return sys::realloc(ptr, layout, new_size);
}

void* System::shrink(void* ptr, Layout layout, usize new_size) {
  if (layout.size <= new_size) {
    return ptr;
  }

  if (new_size == 0) {
    sys::dealloc(ptr, layout);
    return nullptr;
  }

  return sys::realloc(ptr, layout, new_size);
}

}  // namespace sfc::alloc
