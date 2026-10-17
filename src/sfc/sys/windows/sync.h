#pragma once

#include <Windows.h>

#include "sfc/core.h"

namespace sfc::sys::windows {

struct Mutex {
  SRWLOCK _raw;

 public:
  void init();
  void destroy();

 public:
  void lock();
  void unlock();
  auto try_lock() -> bool;
};

struct Condvar {
  CONDITION_VARIABLE _raw;

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
