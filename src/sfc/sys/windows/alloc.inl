#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::windows {

static inline void* alloc(alloc::Layout layout) {
  return ::HeapAlloc(::GetProcessHeap(), 0, layout.size);
}

static inline void dealloc(void* addr, alloc::Layout layout) {
  (void)layout;
  ::HeapFree(::GetProcessHeap(), 0, addr);
}

static inline void* realloc(void* ptr, alloc::Layout layout, size_t new_size) {
  (void)layout;
  return ::HeapReAlloc(::GetProcessHeap(), 0, ptr, new_size);
}

}  // namespace sfc::sys::windows
