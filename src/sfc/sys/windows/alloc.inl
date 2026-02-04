#pragma once
#include <Windows.h>

namespace sfc::sys::alloc {

static inline void* malloc(SIZE_T size) {
  const auto heap = ::GetProcessHeap();
  return ::HeapAlloc(heap, 0, size);
}

static inline void free(void* addr) {
  const auto heap = ::GetProcessHeap();
  ::HeapFree(heap, 0, addr);
}

static inline void* realloc(void* ptr, SIZE_T new_size) {
  const auto heap = ::GetProcessHeap();
  return ::HeapReAlloc(heap, 0, ptr, new_size);
}

}  // namespace sfc::sys::alloc
