#include "sfc/alloc/alloc.h"
#include "sfc/sys/alloc.h"

namespace sfc::alloc {

void* System::allocate(Layout layout) {
  if (layout.size == 0) return nullptr;
  return sys::alloc(layout);
}

void* System::allocate_zeroed(Layout layout) {
  if (layout.size == 0) return nullptr;
  return sys::alloc_zeroed(layout);
}

void System::deallocate(void* ptr, Layout layout) {
  if (ptr == nullptr) return;
  sys::dealloc(ptr, layout);
}

void* System::grow(void* ptr, Layout layout, usize new_size) {
  if (layout.size >= new_size) return ptr;
  return sys::realloc(ptr, layout, new_size);
}

void* System::shrink(void* ptr, Layout layout, usize new_size) {
  if (layout.size <= new_size) return ptr;
  return sys::realloc(ptr, layout, new_size);
}

void* FixedBuf::allocate(Layout layout) {
  if (layout.size == 0 || layout.align == 0) {
    return nullptr;
  }

  const auto remain_cap = _buf._len - _end_idx;
  if (remain_cap < layout.size) {
    return nullptr;
  }

  const auto raw_ptr = _buf._ptr + _end_idx;
  const auto pad_ext = (layout.align - (usize)raw_ptr % layout.align) % layout.align;

  if (remain_cap - layout.size < pad_ext) {
    return nullptr;
  }

  const auto aligned_ptr = raw_ptr + pad_ext;
  _end_idx += pad_ext + layout.size;
  return aligned_ptr;
}

void* FixedBuf::allocate_zeroed(Layout layout) {
  auto* p = this->allocate(layout);
  if (p != nullptr) {
    __builtin_memset(p, 0, layout.size);
  }
  return p;
}

void FixedBuf::deallocate(void* ptr, Layout layout) {
  if (ptr == nullptr) {
    return;
  }

  if (this->is_last_allocation(ptr, layout)) {
    _end_idx -= layout.size;
  }
}

void* FixedBuf::grow(void* ptr, Layout layout, usize new_size) {
  // check if need to grow
  if (new_size <= layout.size) {
    return ptr;
  }

  if (this->is_last_allocation(ptr, layout)) {
    const auto remain_cap = _buf._len - _end_idx;
    const auto additional = new_size - layout.size;
    if (remain_cap >= additional) {
      _end_idx += additional;
      return ptr;
    }
  }

  const auto new_layout = Layout{.size = new_size, .align = layout.align};
  const auto new_ptr = this->allocate(new_layout);
  if (ptr && new_ptr) {
    __builtin_memmove(new_ptr, ptr, layout.size);
  }

  return new_ptr;
}

void* FixedBuf::shrink(void* ptr, Layout layout, usize new_size) {
  // check if need to shrink
  if (new_size >= layout.size) {
    return ptr;
  }

  if (this->is_last_allocation(ptr, layout)) {
    const auto shrink_size = layout.size - new_size;
    _end_idx -= shrink_size;
  }

  return ptr;
}

bool FixedBuf::is_last_allocation(void* ptr, Layout layout) const {
  if (ptr == nullptr) {
    return false;
  }

  const auto* start_ptr = ptr::cast<u8>(ptr);
  if (start_ptr < _buf._ptr) {
    return false;
  }

  const auto* end_ptr = start_ptr + layout.size;
  return end_ptr == _buf._ptr + _end_idx;
}

}  // namespace sfc::alloc
