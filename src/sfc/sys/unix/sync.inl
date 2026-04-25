#pragma once

#include "sfc/sys/unix/mod.inl"
#define _SFC_SYS_SYNC_

namespace sfc::sys::unix {

class Mutex {
  friend class Condvar;

  // pthread_mutex_t cannot be moved
  // so we have to allocate it on heap and manage the pointer
  pthread_mutex_t* _raw{nullptr};

 public:
  explicit Mutex() {
    _raw = static_cast<pthread_mutex_t*>(::malloc(sizeof(pthread_mutex_t)));
    if (auto err = ::pthread_mutex_init(_raw, nullptr); err != 0) {
      ::free(_raw);
      _raw = nullptr;
    }
  }

  ~Mutex() {
    if (_raw == nullptr) {
      return;
    }

    // no need to check error here, since after dtor, the mutex is already unusable
    (void)::pthread_mutex_destroy(_raw);
    ::free(_raw);
  }

  Mutex(Mutex&& other) noexcept : _raw{other._raw} {
    other._raw = nullptr;
  }

  Mutex& operator=(Mutex&& other) noexcept {
    if (this != &other) {
      mem::swap(_raw, other._raw);
    }
    return *this;
  }

  void lock() {
    const auto err = ::pthread_mutex_lock(_raw);
    sfc::expect(err == 0, fmt::Args{"sync::Mutex::lock: failed, err={}", err});
  }

  void unlock() {
    const auto err = ::pthread_mutex_unlock(_raw);
    sfc::expect(err == 0, fmt::Args{"sync::Mutex::unlock: failed, err={}", err});
  }

  auto try_lock() -> bool {
    const auto err = ::pthread_mutex_trylock(_raw);
    sfc::expect(err == 0 || err == EBUSY, fmt::Args{"sync::Mutex::try_lock: failed, err={}", err});
    return err == 0;
  }
};

class Condvar {
#ifdef __APPLE__
  // macOS's pthread_cond_t does not support CLOCK_MONOTONIC
  static constexpr auto CLOCK_ID = CLOCK_REALTIME;
#else
  static constexpr auto CLOCK_ID = CLOCK_MONOTONIC;
#endif

  // pthread_cond_t cannot be moved
  // so we have to allocate it on heap and manage the pointer
  pthread_cond_t* _raw{nullptr};

 public:
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
#endif
    }
  };

  explicit Condvar() {
    auto attr = CondAttr{};
    attr.set_clock(CLOCK_ID);

    _raw = static_cast<pthread_cond_t*>(::malloc(sizeof(pthread_cond_t)));
    if (auto err = ::pthread_cond_init(_raw, &attr._raw); err != 0) {
      ::free(_raw);
      _raw = nullptr;
    }
  }

  ~Condvar() {
    if (_raw == nullptr) {
      return;
    }

    // no need to check error here, since after dtor, the condvar is already unusable
    (void)::pthread_cond_destroy(_raw);
    ::free(_raw);
  }

  Condvar(Condvar&& other) noexcept : _raw{other._raw} {
    other._raw = nullptr;
  }

  Condvar& operator=(Condvar&& other) noexcept {
    if (this != &other) {
      mem::swap(_raw, other._raw);
    }
    return *this;
  }

  void notify_one() {
    const auto err = ::pthread_cond_signal(_raw);
    sfc::expect(err == 0, fmt::Args{"sync::Condvar::notify_one: failed, err={}", err});
  }

  void notify_all() {
    const auto err = ::pthread_cond_broadcast(_raw);
    sfc::expect(err == 0, fmt::Args{"sync::Condvar::notify_all: failed, err={}", err});
  }

  void wait(Mutex& mtx) {
    const auto err = ::pthread_cond_wait(_raw, mtx._raw);
    sfc::expect(err == 0, fmt::Args{"sync::Condvar::wait: failed, err={}", err});
  }

  auto wait_timeout(Mutex& mtx, time::Duration dur) -> bool {
    struct timespec ts{};
    if (::clock_gettime(CLOCK_ID, &ts) == -1) {
      const auto err = errno;
      sfc::expect(false, fmt::Args{"sync::Condvar::wait_timeout: clock_gettime failed, err={}", err});
    }

    ts.tv_sec += dur.as_secs();
    ts.tv_nsec += dur.subsec_nanos();
    if (ts.tv_nsec >= time::NANOS_PER_SEC) {
      ts.tv_sec += 1;
      ts.tv_nsec -= time::NANOS_PER_SEC;
    }

    const auto err = ::pthread_cond_timedwait(_raw, mtx._raw, &ts);
    sfc::expect(err == 0 || err == ETIMEDOUT, fmt::Args{"sync::Condvar::wait_timeout: failed, err={}", err});
    return err == 0;
  }
};

}  // namespace sfc::sys::unix
