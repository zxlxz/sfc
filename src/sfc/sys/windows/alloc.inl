#pragma once

#include <Windows.h>

namespace sfc::sys::alloc {

static inline auto heap() -> HANDLE {
  static const auto res = ::GetProcessHeap();
  return res;
}

static inline auto malloc(SIZE_T size) {
  return ::HeapAlloc(heap(), 0, size);
}

static inline auto free(void* addr) {
  ::HeapFree(heap(), 0, addr);
}

static inline auto realloc(void* ptr, SIZE_T new_size) -> void* {
  return ::HeapReAlloc(heap(), 0, ptr, new_size);
}

static inline auto msize(void* addr) -> SIZE_T {
  return ::HeapSize(heap(), 0, addr);
}

}  // namespace sfc::sys::alloc
