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

class Condvar;

class Mutex {
  friend class Condvar;
  _RTL_SRWLOCK _raw;

 public:
  explicit Mutex();
  ~Mutex();

  Mutex(const Mutex&) = delete;
  Mutex& operator=(const Mutex&) = delete;

 public:
  void lock();
  void unlock();
  auto try_lock() -> bool;
};

class Condvar {
  _RTL_CONDITION_VARIABLE _raw;

 public:
  explicit Condvar();
  ~Condvar();

  Condvar(const Condvar&) = delete;
  Condvar& operator=(const Condvar&) = delete;

 public:
  void notify_one();
  void notify_all();
  void wait(Mutex& mtx);
  auto wait_timeout(Mutex& mtx, time::Duration dur) -> bool;
};

}  // namespace sfc::sys::windows
