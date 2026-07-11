#pragma once

#include "sfc/sys/posix/mod.inl"
#define _SFC_SYS_SYNC_

namespace sfc::sys::posix {

class Mutex {
  struct Inn {
    pthread_mutex_t _0;
  };

  Inn* _ptr{nullptr};

 public:
  explicit Mutex() {
    _ptr = new Inn{};
    (void)::pthread_mutex_init(&_ptr->_0, nullptr);
  }

  ~Mutex() {
    if (_ptr == nullptr) return;
    (void)::pthread_mutex_destroy(&_ptr->_0);
    delete _ptr;
  }

  Mutex(Mutex&& other) noexcept : _ptr{other._ptr} {
    other._ptr = nullptr;
  }

  Mutex& operator=(Mutex&& other) noexcept {
    if (this != &other) {
      mem::swap(_ptr, other._ptr);
    }
    return *this;
  }

  auto raw() -> pthread_mutex_t* {
    return &_ptr->_0;
  }

  void lock() {
    const auto err = ::pthread_mutex_lock(&_ptr->_0);
    sfc::assert_(err == 0, "sync::Mutex::lock: failed, err={}", err);
  }

  void unlock() {
    const auto err = ::pthread_mutex_unlock(&_ptr->_0);
    sfc::assert_(err == 0, "sync::Mutex::unlock: failed, err={}", err);
  }

  auto try_lock() -> bool {
    const auto err = ::pthread_mutex_trylock(&_ptr->_0);
    sfc::assert_(err == 0 || err == EBUSY, "sync::Mutex::try_lock: failed, err={}", err);
    return err == 0;
  }
};

struct CondAttr {
  pthread_condattr_t _raw;

  CondAttr() {
    (void)pthread_condattr_init(&_raw);
  }

  ~CondAttr() {
    (void)pthread_condattr_destroy(&_raw);
  }

  void set_clock(u32 clock_id) {
#ifdef __linux__
    (void)pthread_condattr_setclock(&_raw, clock_id);
#else
    // macos system does not support setting clock for condvar, so just ignore the argument
    (void)clock_id;
#endif
  }
};

class Condvar {
#ifdef __APPLE__
  // macOS's pthread_cond_t does not support CLOCK_MONOTONIC
  static constexpr auto CLOCK_ID = CLOCK_REALTIME;
#else
  static constexpr auto CLOCK_ID = CLOCK_MONOTONIC;
#endif

  struct Inn {
    pthread_cond_t _0;
  };
  Inn* _ptr{nullptr};

 public:
  explicit Condvar() {
    auto attr = CondAttr{};
    attr.set_clock(CLOCK_ID);

    _ptr = new Inn{};
    (void)::pthread_cond_init(&_ptr->_0, &attr._raw);
  }

  ~Condvar() {
    if (_ptr == nullptr) return;
    (void)::pthread_cond_destroy(&_ptr->_0);
    delete _ptr;
  }

  Condvar(Condvar&& other) noexcept : _ptr{other._ptr} {
    other._ptr = nullptr;
  }

  Condvar& operator=(Condvar&& other) noexcept {
    if (this != &other) {
      mem::swap(_ptr, other._ptr);
    }
    return *this;
  }

  void notify_one() {
    const auto err = ::pthread_cond_signal(&_ptr->_0);
    sfc::assert_(err == 0, "sync::Condvar::notify_one: failed, err={}", err);
  }

  void notify_all() {
    const auto err = ::pthread_cond_broadcast(&_ptr->_0);
    sfc::assert_(err == 0, "sync::Condvar::notify_all: failed, err={}", err);
  }

  void wait(Mutex& mtx) {
    const auto err = ::pthread_cond_wait(&_ptr->_0, mtx.raw());
    sfc::assert_(err == 0, "sync::Condvar::wait: failed, err={}", err);
  }

  auto wait_timeout(Mutex& mtx, time::Duration dur) -> bool {
    struct timespec ts{};
    if (::clock_gettime(CLOCK_ID, &ts) == -1) {
      const auto err = errno;
      sfc::assert_(false, "sync::Condvar::wait_timeout: clock_gettime failed, err={}", err);
    }

    ts.tv_sec += dur.as_secs();
    ts.tv_nsec += dur.subsec_nanos();
    if (ts.tv_nsec >= long(time::NANOS_PER_SEC)) {
      ts.tv_sec += 1;
      ts.tv_nsec -= long(time::NANOS_PER_SEC);
    }

    const auto err = ::pthread_cond_timedwait(&_ptr->_0, mtx.raw(), &ts);
    sfc::assert_(err == 0 || err == ETIMEDOUT, "sync::Condvar::wait_timeout: failed, err={}", err);
    return err == 0;
  }
};

}  // namespace sfc::sys::posix
