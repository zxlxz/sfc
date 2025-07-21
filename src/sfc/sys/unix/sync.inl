#pragma once

#include <pthread.h>
#include <time.h>

namespace sfc::sys::sync {

using mutex_t = pthread_mutex_t;
using cond_t = pthread_cond_t;

inline void lock(mutex_t& mtx) {
  ::pthread_mutex_lock(mtx);
}

inline void unlock(mutex_t& mtx) {
  ::pthread_mutex_unlock(&mtx);
}

inline void notify_one(cond_t& cond) {
  ::pthread_cond_signal(&cond);
}

inline void notify_all(cond_t& cond) {
  ::pthread_cond_broadcast(&cond);
}

inline void wait(cond_t& cond, mutex_t& mtx) {
  ::pthread_cond_wait(&cond, &mtx);
}

inline auto wait_timeout_ns(cond_t& cond, mutex_t& mtx, size_t nanos) -> bool {
  static constexpr auto NANOS_PER_SEC = 1000000000U;

  auto ts = timespec_t{};
  ::clock_gettime(CLOCK_MONOTONIC, &ts);

  ts.tv_sec += nanos / NANOS_PER_SEC;
  ts.tv_nsec += nanos % NANOS_PER_SEC;

  const auto err = ::pthread_cond_timedwait(&_raw, &mtx._raw, &ts_wait);
  return err == 0;
}

}  // namespace sfc::sys::sync
