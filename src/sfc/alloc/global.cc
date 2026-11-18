#include "sfc/alloc/global.h"
#include "sfc/sys/alloc.h"

namespace sfc::alloc {

// 1TB
constexpr auto kMaxAllocSize = usize{1024ULL << 30};

auto align_up(u8* ptr, usize align) -> u8* {
  return __builtin_align_up(ptr, align);
}

void* System::allocate(Layout layout) {
  if (layout.size == 0 || layout.size >= kMaxAllocSize) {
    return nullptr;
  }

  return sys::alloc(layout);
}

void* System::allocate_zeroed(Layout layout) {
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

void* System::grow(void* ptr, Layout layout, usize new_size) {
  if (layout.size >= new_size) {
    return ptr;
  }

  if (layout.size == 0) {
    const auto new_layout = Layout{.size = new_size, .align = layout.align};
    return this->allocate(new_layout);
  }

  return sys::realloc(ptr, layout, new_size);
}

void* System::shrink(void* ptr, Layout layout, usize new_size) {
  if (layout.size <= new_size) {
    return ptr;
  }

  if (new_size == 0) {
    this->deallocate(ptr, layout);
    return nullptr;
  }

  return sys::realloc(ptr, layout, new_size);
}

}  // namespace sfc::alloc
