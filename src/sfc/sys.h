#pragma once

namespace sfc::sys {

#ifdef _WIN32
using RawFd = void*;
#else
using RawFd = int;
#endif

#ifndef _SFC_SYS_IO_
struct File {
#ifdef _WIN32
  RawFd _fd = nullptr;
#else
  RawFd _fd = -1;
#endif
};
#endif

#ifndef _SFC_SYS_THREAD_
struct Thread {
#ifdef __unix__
  unsigned long _raw;
#else
  void* _raw;
#endif
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

}  // namespace sfc::sys
