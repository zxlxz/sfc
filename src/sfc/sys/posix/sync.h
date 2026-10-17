#pragma once

#include "sfc/core.h"

namespace sfc::sys::posix {

struct _opaque_mutex_t {
#if defined(__APPLE__)  // apple
  long __sig;
  char __opaque[56];
#elif defined(__linux__)  // linux
  char __opaque[40];
#endif
};

struct _opaque_cond_t {
#if defined(__APPLE__)  // apple
  long __sig;
  char __opaque[40];
#elif defined(__linux__)  // linux
  char __opaque[48];
#endif
};

struct Mutex {
#ifdef _SFC_SYNC_IMP_
  pthread_mutex_t _raw;
#else
  _opaque_mutex_t _raw;
#endif

 public:
  void init();
  void destroy();

 public:
  void lock();
  void unlock();
  auto try_lock() -> bool;
};

class Condvar {
#ifdef _SFC_SYNC_IMP_
  pthread_cond_t _raw;
#else
  _opaque_cond_t _raw;
#endif

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
