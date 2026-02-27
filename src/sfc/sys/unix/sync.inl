#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::sync {

using tid_t = pthread_t;
using mtx_t = pthread_mutex_t;
using cnd_t = pthread_cond_t;
using timespec_t = struct ::timespec;

inline auto get_tid() -> pthread_t {
  return ::pthread_self();
}

inline void mtx_init(pthread_mutex_t& mtx) {
  ::pthread_mutex_init(&mtx, nullptr);
}

inline void mtx_destroy(pthread_mutex_t& mtx) {
  ::pthread_mutex_destroy(&mtx);
}

inline void mtx_lock(pthread_mutex_t& mtx) {
  ::pthread_mutex_lock(&mtx);
}

inline void mtx_unlock(pthread_mutex_t& mtx) {
  ::pthread_mutex_unlock(&mtx);
}

inline bool mtx_trylock(pthread_mutex_t& mtx) {
  const auto ret = ::pthread_mutex_trylock(&mtx);
  return ret == 0;
}

inline void cnd_init(pthread_cond_t& cond) {
  cond = PTHREAD_COND_INITIALIZER;
}

inline void cnd_destroy(pthread_cond_t& cond) {
  (void)cond;
}

inline void cnd_signal(pthread_cond_t& cond) {
  ::pthread_cond_signal(&cond);
}

inline void cnd_broadcast(pthread_cond_t& cond) {
  ::pthread_cond_broadcast(&cond);
}

inline void cnd_wait(pthread_cond_t& cond, pthread_mutex_t& mtx) {
  ::pthread_cond_wait(&cond, &mtx);
}

inline auto cnd_timedwait(pthread_cond_t& cond, pthread_mutex_t& mtx, unsigned millis) -> bool {
  static constexpr auto MILLIS_PER_SEC = 1000U;
  static constexpr auto NANOS_PER_MILLI = 1000000U;

  auto ts = timespec_t{};
  ::clock_gettime(CLOCK_MONOTONIC, &ts);

  ts.tv_sec += millis / MILLIS_PER_SEC;
  ts.tv_nsec += (millis % MILLIS_PER_SEC) * NANOS_PER_MILLI;

  const auto err = ::pthread_cond_timedwait(&cond, &mtx, &ts);
  return err == 0;
}

}  // namespace sfc::sys::.sync
