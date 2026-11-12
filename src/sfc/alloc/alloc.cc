#include "sfc/alloc/alloc.h"
#include "sfc/sys/alloc.h"

namespace sfc::alloc {

// 1TB
constexpr auto kMaxAllocSize = usize{1024ULL << 30};

auto next_power_of_two(usize val) -> usize {
  static constexpr usize kMaxPow2 = usize{1} << (sizeof(usize) * 8 - 1);
  if (val == 0 || val > kMaxPow2) {
    return 0;
  }
  if (val == 1) {
    return 1;
  }

  const auto x = val - 1;
  const auto y = kMaxPow2 >> (__builtin_clzll(x) - 1);
  return y;
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
