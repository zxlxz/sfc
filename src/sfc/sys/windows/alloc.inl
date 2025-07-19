#pragma once

#include <Windows.h>

namespace sfc::sys::alloc {

static inline auto malloc(SIZE_T size) {
  static auto proc = ::GetProcessHeap();
  const auto  addr = ::HeapAlloc(proc, 0, size);
  return addr;
}

static inline auto free(void* addr) {
  static auto proc = ::GetProcessHeap();
  ::HeapFree(proc, 0, addr);
}

static inline auto realloc(void* ptr, SIZE_T new_size) -> void* {
  static auto proc = ::GetProcessHeap();
  const auto  addr = ::HeapReAlloc(proc, 0, ptr, new_size);
  return addr;
}

static inline auto msize(void* addr) -> size_t {
  static auto proc = ::GetProcessHeap();
  const auto  size = ::HeapSize(proc, 0, addr);
  return size;
}

}  // namespace sfc::sys::alloc
