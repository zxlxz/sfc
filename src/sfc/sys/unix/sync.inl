#pragma once

#include <pthread.h>
#include <sys/time.h>

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

  auto wait_timeout_ms(Mutex& mtx, uint32_t millis) -> bool {
    struct timeval now {};
    struct timespec timeout {};
    ::gettimeofday(&now, nullptr);

    timeout.tv_sec = now.tv_sec + static_cast<time_t>(dur.as_secs());
    timeout.tv_nsec = now.tv_usec + static_cast<suseconds_t>(dur.subsec_nanos());

    const auto ret = ::pthread_cond_timedwait(&_raw, &mtx._raw, &timeout);
    return ret == 0;
  }
};

}  // namespace sfc::sys::sync
