#pragma once
#include "sfc/sys/windows/mod.inl"
#define _SFC_SYS_SYNC_

namespace sfc::sys::windows {

struct Mutex {
  SRWLOCK _raw;

 public:
  explicit Mutex() {
    ::InitializeSRWLock(&_raw);
  }

  void lock() {
    ::AcquireSRWLockExclusive(&_raw);
  }

  void unlock() {
    ::ReleaseSRWLockExclusive(&_raw);
  }

  auto try_lock() -> bool {
    return ::TryAcquireSRWLockExclusive(&_raw);
  }
};

struct Condvar {
  CONDITION_VARIABLE _raw;

 public:
  explicit Condvar() {
    ::InitializeConditionVariable(&_raw);
  }

  void notify_one() {
    ::WakeConditionVariable(&_raw);
  }

  void notify_all() {
    ::WakeAllConditionVariable(&_raw);
  }

  void wait(Mutex& mtx) {
    ::SleepConditionVariableSRW(&_raw, &mtx._raw, INFINITE, 0);
  }

  bool wait_timeout(Mutex& mtx, time::Duration dur) {
    const auto ms = static_cast<u32>(dur.as_millis());
    const auto ret = ::SleepConditionVariableSRW(&_raw, &mtx._raw, ms, 0);
    return bool(ret);
  }
};

}  // namespace sfc::sys::windows
