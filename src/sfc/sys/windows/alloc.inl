#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::windows {

static inline void* malloc(mem::Layout layout) {
  if (layout.size == 0) {
    return nullptr;
  }

  return ::HeapAlloc(::GetProcessHeap(), 0, layout.size);
}

static inline void dealloc(void* addr, [[maybe_unused]] mem::Layout layout) noexcept {
  (void)::HeapFree(::GetProcessHeap(), 0, addr);
}

static inline void* realloc(void* ptr, mem::Layout layout, SIZE_T new_size) {
  return ::HeapReAlloc(::GetProcessHeap(), 0, ptr, new_size);
}

}  // namespace sfc::sys::windows
