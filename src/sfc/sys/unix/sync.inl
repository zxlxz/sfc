#pragma once

#include "sfc/sys/unix/mod.inl"
#define _SFC_SYS_SYNC_

namespace sfc::sys::unix {

static constexpr auto MILLIS_PER_SEC = 1000U;
static constexpr auto NANOS_PER_MILLI = 1000000U;

class Mutex {
  friend class Condvar;
  pthread_mutex_t* _raw{nullptr};

 public:
  explicit Mutex() {
    _raw = new pthread_mutex_t{};
    ::pthread_mutex_init(_raw, nullptr);
  }

  ~Mutex() {
    if (!_raw) return;
    ::pthread_mutex_destroy(_raw);
    delete _raw;
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
    if (!_raw) return;
    ::pthread_mutex_lock(_raw);
  }

  void unlock() {
    if (!_raw) return;
    ::pthread_mutex_unlock(_raw);
  }

  auto try_lock() -> bool {
    if (!_raw) return false;
    const auto ret = ::pthread_mutex_trylock(_raw);
    return ret == 0;
  }
};

class Condvar {
  pthread_cond_t* _cond{nullptr};

 public:
  explicit Condvar() {
    _cond = static_cast<pthread_cond_t*>(malloc(sizeof(pthread_cond_t)));
    ::pthread_cond_init(_cond, nullptr);
  }

  ~Condvar() {
    if (!_cond) return;
    ::pthread_cond_destroy(_cond);
    ::free(_cond);
  }

  Condvar(Condvar&& other) noexcept : _cond{other._cond} {
    other._cond = nullptr;
  }

  Condvar& operator=(Condvar&& other) noexcept {
    if (this != &other) {
      mem::swap(_cond, other._cond);
    }
    return *this;
  }

  void notify_one() {
    if (!_cond) return;
    ::pthread_cond_signal(_cond);
  }

  void notify_all() {
    if (!_cond) return;
    ::pthread_cond_broadcast(_cond);
  }

  void wait(Mutex& mtx) {
    if (!_cond) return;
    ::pthread_cond_wait(_cond, mtx._raw);
  }

  bool wait_timeout(Mutex& mtx, u32 millis) {
    if (!_cond) return false;

    struct timespec ts{};
    ::clock_gettime(CLOCK_MONOTONIC, &ts);
    ts.tv_sec += millis / MILLIS_PER_SEC;
    ts.tv_nsec += (millis % MILLIS_PER_SEC) * NANOS_PER_MILLI;

    const auto err = ::pthread_cond_timedwait(_cond, mtx._raw, &ts);
    return err == 0;
  }
};

}  // namespace sfc::sys::unix
