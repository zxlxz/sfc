#include "rc/sys/windows.inl"

namespace rc::sys::windows::alloc {

static constexpr u32 MIN_ALIAN = 8;

struct Header {
  void* _0;
};

static auto _get_header(void* p) -> Header* {
  const auto ret = ptr::cast<Header>(p) - 1;
  return ret;
}

static auto _align_ptr(void* p, usize align) -> void* {
  auto aligned = ptr::cast<u8>(p) + (usize(p) & (align - 1));
  *_get_header(aligned) = Header{p};
  return aligned;
}

static auto _alloc_with_flags(Layout layout, u32 flags) -> void* {
  static auto heap = ::GetProcessHeap();
  if (layout._align <= MIN_ALIAN) {
    const auto res = ::HeapAlloc(heap, flags, layout._size);
    return res;
  }

  const auto size = layout._size + layout._align;
  const auto p = ::HeapAlloc(heap, flags, size);
  if (p == nullptr) {
    return nullptr;
  } else {
    return sys::alloc::_align_ptr(p, layout._align);
  }
}

auto System::alloc(Layout layout) -> void* {
  const auto ret = sys::alloc::_alloc_with_flags(layout, 0);
  return ret;
}

auto System::alloc_zeroed(Layout layout) -> void* {
  const auto ret = sys::alloc::_alloc_with_flags(layout, HEAP_ZERO_MEMORY);
  return ret;
}

auto System::dealloc(void* p, Layout layout) -> void {
  static auto heap = ::GetProcessHeap();

  if (layout._align <= MIN_ALIAN) {
    ::HeapFree(heap, 0, p);
  } else {
    const auto header = ptr::cast<Header*>(p) - 1;
    ::HeapFree(heap, 0, header);
  }
}

auto System::realloc(void* p, Layout layout, usize new_size) -> void* {
  static auto heap = ::GetProcessHeap();
  if (layout._align <= MIN_ALIAN) {
    const auto res = ::HeapReAlloc(heap, 0, p, new_size);
    return res;
  } else {
    const auto res = System::alloc(layout);
    __builtin_memcpy(res, p, layout._size);
    System::dealloc(p, layout);
    return res;
  }
}

}  // namespace rc::sys::windows::alloc
