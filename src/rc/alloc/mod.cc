#include "rc.inl"

#include "rc/alloc.h"
#include "rc/sys.h"

namespace rc::alloc {

pub auto Global::alloc(Layout layout) -> void* {
  if (layout._size == 0) return nullptr;
  return sys::alloc::System::alloc(layout);
}

pub auto Global::alloc_zeroed(Layout layout) -> void* {
  if (layout._size == 0) return nullptr;
  return sys::alloc::System::alloc_zeroed(layout);
}

pub auto Global::dealloc(void* p, Layout layout) -> void {
  if (p == nullptr) return;
  return sys::alloc::System::dealloc(p, layout);
}

pub auto Global::realloc(void* p, Layout layout, usize new_size) -> void* {
  if (p == nullptr) {
    return Global::alloc({layout._align, new_size});
  }
  if (new_size == 0) {
    Global::dealloc(p, layout);
    return nullptr;
  }
  return sys::alloc::System::realloc(p, layout, new_size);
}

}  // namespace rc::alloc
