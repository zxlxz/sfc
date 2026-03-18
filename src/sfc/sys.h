#pragma once

#include "sfc/core.h"

namespace sfc::sys {

#ifdef _WIN32
using RawFd = void*;
#else
using RawFd = int;
#endif

#ifndef _SFC_SYS_IO_
struct File {
  RawFd _raw;
};
#endif

#ifndef _SFC_SYS_THREAD_
struct Thread {
  void* _raw;
};
#endif

#ifndef _SFC_SYS_SYNC_
struct Mutex {
  void* _raw;
};

struct Condvar {
  void* _raw;
};
#endif

}  // namespace sfc::sys::raw
