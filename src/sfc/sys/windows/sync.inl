#pragma once
#include "sfc/sys/windows/mod.inl"
#define _SFC_SYS_SYNC_

namespace sfc::sys::windows {

class Mutex {
  struct Inn {
    SRWLOCK _0;
  };
  Inn* _ptr{nullptr};

 public:
  explicit Mutex() {
    _ptr = new Inn{};
    ::InitializeSRWLock(&_ptr->_0);
  }

  ~Mutex() {
    if (_ptr == nullptr) return;
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

  auto raw() -> SRWLOCK* {
    return &_ptr->_0;
  }

  void lock() {
    ::AcquireSRWLockExclusive(&_ptr->_0);
  }

  void unlock() {
    ::ReleaseSRWLockExclusive(&_ptr->_0);
  }

  auto try_lock() -> bool {
    const auto ret = ::TryAcquireSRWLockExclusive(&_ptr->_0);
    return bool(ret);
  }
};

class Condvar {
  struct Inn {
    CONDITION_VARIABLE _0;
  };
  Inn* _ptr{nullptr};

 public:
  explicit Condvar() {
    _ptr = new Inn{};
    ::InitializeConditionVariable(&_ptr->_0);
  }

  ~Condvar() {
    if (_ptr == nullptr) return;
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
    ::WakeConditionVariable(&_ptr->_0);
  }

  void notify_all() {
    ::WakeAllConditionVariable(&_ptr->_0);
  }

  void wait(Mutex& mtx) {
    (void)::SleepConditionVariableSRW(&_ptr->_0, mtx.raw(), INFINITE, 0);
  }

  bool wait_timeout(Mutex& mtx, time::Duration dur) {
    const auto ms = num::saturating_cast<u32>(dur.as_millis());
    const auto ret = ::SleepConditionVariableSRW(&_ptr->_0, mtx.raw(), ms, 0);
    return bool(ret);
  }
};

}  // namespace sfc::sys::windows
