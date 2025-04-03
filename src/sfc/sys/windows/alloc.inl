#pragma once

#include <malloc.h>

namespace sfc::sys::alloc {

using ::malloc;
using ::free;
using ::realloc;

static inline auto msize(void* p) -> size_t {
  return ::_msize(p);
}

}  // namespace sfc::sys::alloc
