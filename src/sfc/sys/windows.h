#pragma once

#include "sfc/core.h"

namespace sfc::sys::windows {

using RawFd = void*;

#ifndef _SFC_SYS_IO_
class File {
  void* _handle;

 public:
  File();
  ~File();

  File(File&& other) noexcept;
  File& operator=(File&& other) noexcept;
};
#endif

#ifndef _SFC_SYS_THREAD_
struct Thread {
  void* _handle;
};
#endif

#ifndef _SFC_SYS_SYNC_
class Mutex {
  struct Inn;
  Inn* _ptr;

 public:
  explicit Mutex();
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

}  // namespace sfc::sys::windows

namespace sfc::sys {
using windows::RawFd;
using windows::File;

using windows::Thread;

using windows::Condvar;
using windows::Mutex;
}  // namespace sfc::sys
