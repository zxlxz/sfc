#pragma once
#if defined(__unix__) || defined(__APPLE__)

#include <pthread.h>
#include <sys/time.h>
#include <time.h>

namespace sfc::sys::sync {

using tid_t = pthread_t;
using mtx_t = pthread_mutex_t;
using cnd_t = pthread_cond_t;
using timespec_t = struct ::timespec;

inline auto get_tid() -> tid_t {
  return ::pthread_self();
}

inline void mtx_init(mtx_t& mtx) {
  mtx = PTHREAD_MUTEX_INITIALIZER;
}

inline void mtx_destroy(mtx_t& mtx) {
  (void)mtx;
}

inline void mtx_lock(mtx_t& mtx) {
  ::pthread_mutex_lock(&mtx);
}

inline void mtx_unlock(mtx_t& mtx) {
  ::pthread_mutex_unlock(&mtx);
}

inline bool mtx_trylock(mtx_t& mtx) {
  const auto ret = ::pthread_mutex_trylock(&mtx);
  return ret == 0;
}

inline void cnd_init(cnd_t& cond) {
  cond = PTHREAD_COND_INITIALIZER;
}

inline void cnd_destroy(cnd_t& cond) {
  (void)cond;
}

inline void cnd_signal(cnd_t& cond) {
  ::pthread_cond_signal(&cond);
}

inline void cnd_broadcast(cnd_t& cond) {
  ::pthread_cond_broadcast(&cond);
}

inline void cnd_wait(cnd_t& cond, mtx_t& mtx) {
  ::pthread_cond_wait(&cond, &mtx);
}

inline auto cnd_timedwait(cnd_t& cond, mtx_t& mtx, unsigned millis) -> bool {
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
