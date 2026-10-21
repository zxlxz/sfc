#pragma once

#include "sfc/core.h"

#ifndef _WINDOWS_
struct _RTL_SRWLOCK {
  void* Ptr;
};

struct _RTL_CONDITION_VARIABLE {
  void* Ptr;
};
#endif

namespace sfc::sys::windows {

struct Mutex {
  _RTL_SRWLOCK _raw;

 public:
  void init();
  void destroy();

 public:
  void lock();
  void unlock();
  auto try_lock() -> bool;
};

struct Condvar {
  _RTL_CONDITION_VARIABLE _raw;

 public:
  void init();
  void destroy();

 public:
  void notify_one();
  void notify_all();
  void wait(Mutex& mtx);
  auto wait_timeout(Mutex& mtx, time::Duration dur) -> bool;
};

}  // namespace sfc::sys::windows
