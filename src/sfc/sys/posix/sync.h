#pragma once

#include <pthread.h>
#include "sfc/core.h"

namespace sfc::sys::posix {

class Condvar;

class Mutex {
  friend class Condvar;
  pthread_mutex_t _raw;

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
  pthread_cond_t _raw;

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

}  // namespace sfc::sys::posix
