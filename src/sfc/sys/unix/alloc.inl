#pragma once

#include <malloc.h>

namespace sfc::sys::alloc {

using ::malloc;
using ::free;
using ::realloc;

static inline auto msize(void* p) -> size_t {
  return ::malloc_usable_size(p);
}

}  // namespace sfc::sys::alloc
