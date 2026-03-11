#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::windows {

struct Mutex : SRWLOCK {
  explicit Mutex() {
    ::InitializeSRWLock(this);
  }

  void lock() {
    ::AcquireSRWLockExclusive(this);
  }

  void unlock() {
    ReleaseSRWLockExclusive(this);
  }

  auto try_lock() -> bool {
    return ::TryAcquireSRWLockExclusive(this);
  }
};

struct Condvar : CONDITION_VARIABLE {
  explicit Condvar() {
    ::InitializeConditionVariable(this);
  }

  void notify_one() {
    ::WakeConditionVariable(this);
  }

  void notify_all() {
    ::WakeAllConditionVariable(this);
  }

  void wait(Mutex& mtx) {
    ::SleepConditionVariableSRW(this, &mtx, INFINITE, 0);
  }

  bool wait_timeout(Mutex& mtx, DWORD millis) {
    const auto ret = ::SleepConditionVariableSRW(this, &mtx, millis, 0);
    return bool(ret);
  }
};

}  // namespace sfc::sys::windows
