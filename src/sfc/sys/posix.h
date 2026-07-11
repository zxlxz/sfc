#pragma once

#include "sfc/core.h"

namespace sfc::sys::posix {

using RawFd = int;

#ifndef _SFC_SYS_IO_
class File {
  int _fd = -1;

 public:
  File();
  ~File();

  File(File&& other) noexcept;
  File& operator=(File&& other) noexcept;
};
#endif

#ifndef _SFC_SYS_THREAD_
struct Thread {
#ifdef __APPLE__
  void* _raw;
#else
  unsigned long _raw;
#endif
};
#endif

#ifndef _SFC_SYS_SYNC_
class Mutex {
  struct Inn;
  Inn* _ptr;

 public:
  Mutex();
  ~Mutex();

  Mutex(Mutex&& other) noexcept;
  Mutex& operator=(Mutex&& other) noexcept;
};

class Condvar {
  struct Inn;
  Inn* _ptr;

 public:
  Condvar();
  ~Condvar();

  Condvar(Condvar&& other) noexcept;
  Condvar& operator=(Condvar&& other) noexcept;
};
#endif

}  // namespace sfc::sys::posix

namespace sfc::sys {
using posix::RawFd;
using posix::File;

using posix::Thread;

using posix::Condvar;
using posix::Mutex;
}  // namespace sfc::sys
