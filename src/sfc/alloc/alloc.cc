#include "sfc/alloc/alloc.h"
#include "sfc/sys/alloc.h"

namespace sfc::alloc {

namespace {
// align must be a power of two
auto padding_size(void* ptr, usize align) noexcept -> usize {
  if (align <= 1) return 0;
  return (~(usize)ptr + 1) & (align - 1);
}
}  // namespace

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
  if (layout.size == 0) return nullptr;

  const auto end_ptr = _buf.ptr() + _end_idx;
  const auto pad_size = alloc::padding_size(end_ptr, layout.align);

  // check if has enough space for the allocation including padding
  if (pad_size + layout.size > _buf.len() - _end_idx) {
    return nullptr;
  }

  _end_idx += pad_size + layout.size;
  return end_ptr + pad_size;
}

void* FixedBuf::allocate_zeroed(Layout layout) {
  if (layout.size == 0) return nullptr;

  const auto p = this->allocate(layout);
  if (p != nullptr) {
    ptr::write_bytes((u8*)p, 0, layout.size);
  }
  return p;
}

void FixedBuf::deallocate(void* ptr, Layout layout) {
  if (!this->is_last_allocation(ptr, layout)) return;

  // the alignment padding before `ptr` is not reclaimed, same as bumpalo
  _end_idx -= layout.size;
}

void* FixedBuf::grow(void* ptr, Layout layout, usize new_size) {
  if (new_size <= layout.size) return ptr;

  if (this->is_last_allocation(ptr, layout)) {
    // grow in place if there is enough remaining space
    const auto add_size = new_size - layout.size;
    if (add_size > _buf.len() - _end_idx) return nullptr;
    _end_idx += add_size;
    return ptr;
  }

  const auto new_layout = Layout{.size = new_size, .align = layout.align};
  const auto new_ptr = this->allocate(new_layout);
  if (new_ptr != nullptr) {
    ptr::copy((const u8*)ptr, (u8*)new_ptr, layout.size);
    this->deallocate(ptr, layout);
  }
  return new_ptr;
}

void* FixedBuf::shrink(void* ptr, Layout layout, usize new_size) {
  if (new_size >= layout.size) return ptr;

  if (this->is_last_allocation(ptr, layout)) {
    _end_idx -= layout.size - new_size;
  }

  return ptr;
}

bool FixedBuf::is_last_allocation(void* ptr, Layout layout) const {
  if (ptr == nullptr) return false;

  const auto ptr_start = (const u8*)ptr;
  const auto ptr_end = ptr_start + layout.size;

  const auto buf_start = _buf.ptr();
  const auto buf_end = buf_start + _end_idx;

  return ptr_start >= buf_start && ptr_end == buf_end;
}

}  // namespace sfc::alloc
