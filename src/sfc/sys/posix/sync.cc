#include <errno.h>
#include <pthread.h>

#include "sfc/sys/posix/sync.h"

namespace sfc::sys::posix {

void Mutex::init() {
  (void)::pthread_mutex_init(&_raw, nullptr);
}

void Mutex::destroy() {
  (void)::pthread_mutex_destroy(&_raw);
}

void Mutex::lock() {
  const auto err = ::pthread_mutex_lock(&_raw);
  sfc::assert_(err == 0, "sync::Mutex::lock: failed, err={}", err);
}

void Mutex::unlock() {
  const auto err = ::pthread_mutex_unlock(&_raw);
  sfc::assert_(err == 0, "sync::Mutex::unlock: failed, err={}", err);
}

auto Mutex::try_lock() -> bool {
  const auto err = ::pthread_mutex_trylock(&_raw);
  sfc::assert_(err == 0 || err == EBUSY, "sync::Mutex::try_lock: failed, err={}", err);
  return err == 0;
}

void Condvar::init() {
  auto attr = pthread_condattr_t{};
  (void)::pthread_condattr_init(&attr);
#ifndef __APPLE__
  (void)::pthread_condattr_setclock(&attr, CLOCK_MONOTONIC);
#endif
  (void)::pthread_cond_init(&_raw, &attr);
  (void)::pthread_condattr_destroy(&attr);
}

void Condvar::destroy() {
  (void)::pthread_cond_destroy(&_raw);
}

void Condvar::notify_one() {
  const auto err = ::pthread_cond_signal(&_raw);
  sfc::assert_(err == 0, "sync::Condvar::notify_one: failed, err={}", err);
}

void Condvar::notify_all() {
  const auto err = ::pthread_cond_broadcast(&_raw);
  sfc::assert_(err == 0, "sync::Condvar::notify_all: failed, err={}", err);
}

void Condvar::wait(Mutex& mtx) {
  const auto err = ::pthread_cond_wait(&_raw, &mtx._raw);
  sfc::assert_(err == 0, "sync::Condvar::wait: failed, err={}", err);
}

auto Condvar::wait_timeout(Mutex& mtx, time::Duration dur) -> bool {
#ifdef __APPLE__
  static constexpr auto kClock = CLOCK_REALTIME;
#else
  static constexpr auto kClock = CLOCK_MONOTONIC;
#endif

  struct timespec sys_time{};
  if (::clock_gettime(kClock, &sys_time) == -1) {
    sfc::assert_(false, "sync::Condvar::wait_timeout: clock_gettime failed, err={}", errno);
  }

  sys_time.tv_sec += dur.as_secs();
  sys_time.tv_nsec += dur.subsec_nanos();
  if (sys_time.tv_nsec >= long(time::NANOS_PER_SEC)) {
    sys_time.tv_sec += 1;
    sys_time.tv_nsec -= long(time::NANOS_PER_SEC);
  }

  const auto err = ::pthread_cond_timedwait(&_raw, &mtx._raw, &sys_time);
  sfc::assert_(err == 0 || err == ETIMEDOUT, "sync::Condvar::wait_timeout: failed, err={}", err);
  return err == 0;
}

}  // namespace sfc::sys::posix
