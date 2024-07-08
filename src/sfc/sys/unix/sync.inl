#pragma once

#include <pthread.h>

namespace sfc::sys::sync {

struct Mutex {
  pthread_mutex_t _raw = PTHREAD_MUTEX_INITIALIZER;

 public:
  void lock() {
    ::pthread_mutex_lock(&_raw);
  }

  void unlock() {
    ::pthread_mutex_unlock(&_raw);
  }
};

struct Condvar {
  pthread_cond_t _raw = PTHREAD_COND_INITIALIZER;

  void notify_one() {
    ::pthread_cond_signal(&_raw);
  }

  void notify_all() {
    ::pthread_cond_broadcast(&_raw);
  }

  void wait(Mutex& mtx) {
    ::pthread_cond_wait(&_raw, &mtx._raw);
  }

  auto wait_timeout(Mutex& mtx, auto dur) -> bool {
    struct timespec ts {};
    ::clock_gettime(CLOCK_REALTIME, &ts);

    ts.tv_sec += static_cast<time_t>(dur.as_secs());
    ts.tv_nsec += static_cast<long>(dur.subsec_nanos());

    const auto ret = ::pthread_cond_timedwait(&_raw, &mtx._raw, &ts);
    return ret == 0;
  }
};

}  // namespace sfc::sys::sync
