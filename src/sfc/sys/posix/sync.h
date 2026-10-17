#pragma once

#include <pthread.h>
#include "sfc/core.h"

namespace sfc::sys::posix {

struct Mutex {
  pthread_mutex_t _raw;

 public:
  void init();
  void destroy();

 public:
  void lock();
  void unlock();
  auto try_lock() -> bool;
};

struct Condvar {
  pthread_cond_t _raw;

 public:
  void init();
  void destroy();

 public:
  void notify_one();
  void notify_all();
  void wait(Mutex& mtx);
  auto wait_timeout(Mutex& mtx, time::Duration dur) -> bool;
};

}  // namespace sfc::sys::posix
