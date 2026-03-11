#pragma once

// posix
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

// unix
#if defined(__unix__) || defined(__APPLE__)
#include <cxxabi.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <pthread.h>
#include <pwd.h>
#include <sys/time.h>
#include <sys/syscall.h>
#include <unistd.h>
#endif

// apple
#if defined(__APPLE__)
#include <mach-o/dyld.h>
#include <_time.h>
#endif

#undef unix
#undef stdin
#undef stdout
#undef stderr

#include "sfc/core.h"
#include "sfc/ffi.h"

namespace sfc::sys::unix {

static inline auto os_error() -> int {
  return errno;
}

}  // namespace sfc::sys::unix

namespace sfc::sys {
using namespace unix;
}
