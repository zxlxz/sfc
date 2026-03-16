#pragma once

#include "sfc/core.h"

namespace sfc::sys::raw {

#ifdef _WIN32
using RawFd = void*;
#else
using RawFd = int;
#endif

struct File {
  RawFd _raw;
};

struct Thread {
  void* _raw;
};

struct Mutex {
  void* _raw;
};

struct Condvar {
  void* _raw;
};

}  // namespace sfc::sys::raw

namespace sfc::sys {

using raw::RawFd;

#ifndef _SFC_SYS_IO_
using raw::File;
#endif

#ifndef _SFC_SYS_THREAD_
using raw::Thread;
#endif

#ifndef _SFC_SYS_SYNC_
using raw::Condvar;
using raw::Mutex;
#endif
}  // namespace sfc::sys
