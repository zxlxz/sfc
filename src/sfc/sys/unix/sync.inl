#pragma once
#if defined(__unix__) || defined(__APPLE__)

#include <pthread.h>
#include <sys/time.h>
#include <time.h>

namespace sfc::sys::sync {

using mutex_t = pthread_mutex_t;
using cond_t = pthread_cond_t;
using timespec_t = struct ::timespec;

inline void init(mutex_t& mtx) {
  mtx = PTHREAD_MUTEX_INITIALIZER;
}

inline void drop(mutex_t& mtx) {
  (void)mtx;
}

inline void lock(mutex_t& mtx) {
  ::pthread_mutex_lock(&mtx);
}

inline void unlock(mutex_t& mtx) {
  ::pthread_mutex_unlock(&mtx);
}

inline void init(cond_t& cond) {
  cond = PTHREAD_COND_INITIALIZER;
}

inline void drop(cond_t& cond) {
  (void)cond;
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

inline auto wait_timeout_ms(cond_t& cond, mutex_t& mtx, unsigned millis) -> bool {
  static constexpr auto MILLIS_PER_SEC = 1000U;
  static constexpr auto NANOS_PER_MILLI = 1000000U;

  auto ts = timespec_t{};
  ::clock_gettime(CLOCK_MONOTONIC, &ts);

  ts.tv_sec += millis / MILLIS_PER_SEC;
  ts.tv_nsec += (millis % MILLIS_PER_SEC) * NANOS_PER_MILLI;

  const auto err = ::pthread_cond_timedwait(&cond, &mtx, &ts);
  return err == 0;
}

}  // namespace sfc::sys::sync
#endif
