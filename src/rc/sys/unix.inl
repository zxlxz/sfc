#pragma once

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>

#if __has_include(<unistd.h>)
#include <unistd.h>
#include <copyfile.h>
#include <execinfo.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include <pthread.h>
#endif

#if __has_include(<mach-o/dyld.h>)
#include <mach-o/dyld.h>
#endif

#if __has_include(<unistd.h>)
#define _rc_SYS_UNIX_IMPL_
#endif

#include "rc/sys/unix.h"

namespace rc::sys::unix {

using namespace rc::time;

inline auto dur2ts(Duration dur) -> timespec {
  const auto ts = ::timespec{time_t(dur._secs), long(dur._nanos)};
  return ts;
}

}  // namespace rc::sys::unix
