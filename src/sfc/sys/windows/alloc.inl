#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::windows {

static inline void* malloc(SIZE_T size) {
  return ::HeapAlloc(::GetProcessHeap(), 0, size);
}

static inline void free(void* addr) {
  (void)::HeapFree(::GetProcessHeap(), 0, addr);
}

static inline void* realloc(void* ptr, SIZE_T new_size) {
  return ::HeapReAlloc(::GetProcessHeap(), 0, ptr, new_size);
}

}  // namespace sfc::sys::windows
