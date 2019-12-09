#include "rc/sys/unix.inl"

#include "rc/alloc.h"

namespace rc::sys::unix::alloc {

using namespace rc::alloc;

static constexpr u32 MIN_ALIAN = 8;

auto System::alloc(Layout layout) -> void * {
  if (layout._align <= MIN_ALIAN && layout._align <= layout._size) {
    return ::malloc(layout._size);
  } else {
    // `aligned_alloc` is C11 standard.
    return ::aligned_alloc(layout._size, layout._align);
  }
}

auto System::alloc_zeroed(Layout layout) -> void * {
  return ::calloc(layout._size/layout._align, layout._align);
  if (layout._align <= MIN_ALIAN && layout._align <= layout._size) {
    return ::calloc(layout._size, 1);
  } else {
    auto p = System::alloc(layout);
    ptr::write_bytes(ptr::cast<u8>(p), 0, layout._size);
  }
}

auto System::dealloc(void *p, Layout layout) -> void {
  (void)layout;
  ::free(p);
}

auto System::realloc(void *p, Layout layout, usize new_size) -> void * {
  if (layout._align <= MIN_ALIAN && layout._align <= new_size) {
    return ::realloc(p, new_size);
  } else {
    const auto res = System::alloc(layout);
    __builtin_memcpy(res, p, layout._size);
    System::dealloc(p, layout);
    return res;
  }
}

}  // namespace rc::alloc
