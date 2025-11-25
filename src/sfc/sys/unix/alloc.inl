#pragma once
#if defined(__unix__) || defined(__APPLE__)

#include <stdlib.h>

#if defined(__APPLE__)
#include <malloc/malloc.h>
#else
#include <malloc.h>
#endif

namespace sfc::sys::alloc {

using ::free;
using ::malloc;
using ::realloc;

static inline auto msize(void* p) -> size_t {
#if defined(__APPLE__)
  return ::malloc_size(p);
#else
  return ::malloc_usable_size(p);
#endif
}

}  // namespace sfc::sys::alloc
#endif
