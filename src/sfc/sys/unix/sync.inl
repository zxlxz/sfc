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
    if (_raw == nullptr) {
      throw io::Error::InvalidData;
    }

    const auto err = ::pthread_mutex_lock(_raw);
    if (err != 0) {
      throw io::from_raw_os_error(err);
    }
  }

  void unlock() {
    if (_raw == nullptr) {
      throw io::Error::InvalidData;
    }

    const auto err = ::pthread_mutex_unlock(_raw);
    if (err != 0) {
      throw io::from_raw_os_error(err);
    }
  }

  auto try_lock() -> bool {
    if (_raw == nullptr) {
      throw io::Error::InvalidData;
    }

    const auto ret = ::pthread_mutex_trylock(_raw);
    if (ret != 0 && ret != EBUSY) {
      throw io::from_raw_os_error(ret);
    }
    return ret == 0;
  }
};

class Condvar {
#ifdef __APPLE__
  // macOS's pthread_cond_t does not support CLOCK_MONOTONIC, and it always uses CLOCK_REALTIME internally
  static constexpr auto CLOCK_ID = CLOCK_REALTIME;
#else
  static constexpr auto CLOCK_ID = CLOCK_MONOTONIC;
#endif

  // pthread_cond_t cannot be moved
  // so we have to allocate it on heap and manage the pointer
  pthread_cond_t* _raw{nullptr};

 public:
  explicit Condvar() {
    auto attr = pthread_condattr_t{};
    if (::pthread_condattr_init(&attr) != 0) {
      return;
    }

#ifdef __linux__
    if (::pthread_condattr_setclock(&attr, CLOCK_MONOTONIC) != 0) {
      return;
    }
#endif

    _raw = static_cast<pthread_cond_t*>(::malloc(sizeof(pthread_cond_t)));
    if (auto err = ::pthread_cond_init(_raw, &attr); err != 0) {
      ::free(_raw);
      _raw = nullptr;
    }

    // no need to check error here, since even if attr destruction fails, it won't affect the condvar
    (void)::pthread_condattr_destroy(&attr);
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
    if (_raw == nullptr) {
      throw io::Error::InvalidData;
    }

    if (auto err = ::pthread_cond_signal(_raw); err != 0) {
      throw io::from_raw_os_error(err);
    }
  }

  void notify_all() {
    if (_raw == nullptr) {
      throw io::Error::InvalidData;
    }

    if (auto err = ::pthread_cond_broadcast(_raw); err != 0) {
      throw io::from_raw_os_error(err);
    }
  }

  void wait(Mutex& mtx) {
    if (_raw == nullptr) {
      throw io::Error::InvalidData;
    }

    if (auto err = ::pthread_cond_wait(_raw, mtx._raw); err != 0) {
      throw io::from_raw_os_error(err);
    }
  }

  bool wait_timeout(Mutex& mtx, time::Duration dur) {
    if (_raw == nullptr) {
      throw io::Error::InvalidData;
    }

    struct timespec ts{};
    if (::clock_gettime(CLOCK_ID, &ts) == -1) {
      throw io::last_os_error();
    }

    ts.tv_sec += dur.as_secs();
    ts.tv_nsec += dur.subsec_nanos();
    if (ts.tv_nsec >= time::NANOS_PER_SEC) {
      ts.tv_sec += 1;
      ts.tv_nsec -= time::NANOS_PER_SEC;
    }

    const auto err = ::pthread_cond_timedwait(_raw, mtx._raw, &ts);
    if (err != 0 && err != ETIMEDOUT) {
      throw io::from_raw_os_error(err);
    }
    return err == 0;
  }
};

}  // namespace sfc::sys::unix
