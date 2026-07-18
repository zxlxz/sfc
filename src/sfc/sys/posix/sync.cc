#include <pthread.h>
#include <errno.h>

#include "sfc/sys/posix.h"

namespace sfc::sys::posix {

struct Mutex::Inn {
  pthread_mutex_t _0;

 public:
  Inn() {
    (void)::pthread_mutex_init(&_0, nullptr);
  }

  ~Inn() {
    (void)::pthread_mutex_destroy(&_0);
  }

  Inn(const Inn&) = delete;
  Inn& operator=(const Inn&) = delete;
};

Mutex::Mutex() {
  _ptr = new Inn{};
}

Mutex::~Mutex() {
  if (_ptr == nullptr) return;
  delete _ptr;
}

Mutex::Mutex(Mutex&& other) noexcept : _ptr{mem::take(other._ptr)} {}

Mutex& Mutex::operator=(Mutex&& other) noexcept {
  if (this != &other) {
    mem::swap(_ptr, other._ptr);
  }
  return *this;
}

auto Mutex::inn() -> Inn& {
  return *_ptr;
}

void Mutex::lock() {
  auto& sys_mtx = _ptr->_0;
  const auto err = ::pthread_mutex_lock(&sys_mtx);
  sfc::assert_(err == 0, "sync::Mutex::lock: failed, err={}", err);
}

void Mutex::unlock() {
  auto& sys_mtx = _ptr->_0;
  const auto err = ::pthread_mutex_unlock(&sys_mtx);
  sfc::assert_(err == 0, "sync::Mutex::unlock: failed, err={}", err);
}

auto Mutex::try_lock() -> bool {
  auto& sys_mtx = _ptr->_0;
  const auto err = ::pthread_mutex_trylock(&sys_mtx);
  sfc::assert_(err == 0 || err == EBUSY, "sync::Mutex::try_lock: failed, err={}", err);
  return err == 0;
}

struct CondvarAttr {
  pthread_condattr_t _0;

 public:
  CondvarAttr() {
    (void)::pthread_condattr_init(&_0);
  }

  ~CondvarAttr() {
    (void)::pthread_condattr_destroy(&_0);
  }

  CondvarAttr(const CondvarAttr&) = delete;
  CondvarAttr& operator=(const CondvarAttr&) = delete;

 public:
  void set_clock([[maybe_unused]] int clock) {
#ifndef __APPLE__
    (void)::pthread_condattr_setclock(&_0, clock);
#endif
  }
};

struct Condvar::Inn {
#ifdef __APPLE__
  static constexpr auto kClock = CLOCK_REALTIME;
#else
  static constexpr auto kClock = CLOCK_MONOTONIC;
#endif

  pthread_cond_t _0;

 public:
  Inn() {
    auto attr = CondvarAttr{};
    attr.set_clock(kClock);
    (void)::pthread_cond_init(&_0, &attr._0);
  }

  ~Inn() {
    (void)::pthread_cond_destroy(&_0);
  }

  Inn(const Inn&) = delete;
  Inn& operator=(const Inn&) = delete;
};

Condvar::Condvar() {
  _ptr = new Inn{};
}

Condvar::~Condvar() {
  if (_ptr == nullptr) return;
  delete _ptr;
}

Condvar::Condvar(Condvar&& other) noexcept : _ptr{mem::take(other._ptr)} {}

Condvar& Condvar::operator=(Condvar&& other) noexcept {
  if (this != &other) {
    mem::swap(_ptr, other._ptr);
  }
  return *this;
}

void Condvar::notify_one() {
  auto& sys_cond = _ptr->_0;
  const auto err = ::pthread_cond_signal(&sys_cond);
  sfc::assert_(err == 0, "sync::Condvar::notify_one: failed, err={}", err);
}

void Condvar::notify_all() {
  auto& sys_cond = _ptr->_0;
  const auto err = ::pthread_cond_broadcast(&sys_cond);
  sfc::assert_(err == 0, "sync::Condvar::notify_all: failed, err={}", err);
}

void Condvar::wait(Mutex& mtx) {
  auto& sys_cond = _ptr->_0;
  auto& sys_mutex = mtx.inn()._0;

  const auto err = ::pthread_cond_wait(&sys_cond, &sys_mutex);
  sfc::assert_(err == 0, "sync::Condvar::wait: failed, err={}", err);
}

auto Condvar::wait_timeout(Mutex& mtx, time::Duration dur) -> bool {
  auto& sys_cond = _ptr->_0;
  auto& sys_mutex = mtx.inn()._0;

  struct timespec sys_time{};
  if (::clock_gettime(Inn::kClock, &sys_time) == -1) {
    const auto err = errno;
    sfc::assert_(false, "sync::Condvar::wait_timeout: clock_gettime failed, err={}", err);
  }

  sys_time.tv_sec += dur.as_secs();
  sys_time.tv_nsec += dur.subsec_nanos();
  if (sys_time.tv_nsec >= long(time::NANOS_PER_SEC)) {
    sys_time.tv_sec += 1;
    sys_time.tv_nsec -= long(time::NANOS_PER_SEC);
  }

  const auto err = ::pthread_cond_timedwait(&sys_cond, &sys_mutex, &sys_time);
  sfc::assert_(err == 0 || err == ETIMEDOUT, "sync::Condvar::wait_timeout: failed, err={}", err);
  return err == 0;
}

}  // namespace sfc::sys::posix
