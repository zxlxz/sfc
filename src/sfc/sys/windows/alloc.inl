#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::windows {

using alloc::Layout;

static inline void* alloc(Layout layout) {
  return ::HeapAlloc(::GetProcessHeap(), 0, layout.size);
}

static inline void dealloc(void* addr, [[maybe_unused]] Layout layout) {
  (void)layout;
  (void)::HeapFree(::GetProcessHeap(), 0, addr);
}

static inline void* realloc(void* ptr, [[maybe_unused]] Layout layout, size_t new_size) {
  return ::HeapReAlloc(::GetProcessHeap(), 0, ptr, new_size);
}

}  // namespace sfc::sys::windows
