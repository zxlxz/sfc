#pragma once

#include <Windows.h>

namespace sfc::sys::sync {

struct Mutex {
  ::CRITICAL_SECTION _cs{};

 public:
  Mutex() {
    ::InitializeCriticalSection(&_cs);
  }

  ~Mutex() {
    ::DeleteCriticalSection(&_cs);
  }

  Mutex(const Mutex&) = delete;

  void operator=(const Mutex&) = delete;

  void lock() {
    ::EnterCriticalSection(&_cs);
  }

  void unlock() {
    ::LeaveCriticalSection(&_cs);
  }
};

struct Condvar {
  ::CONDITION_VARIABLE _cv{};

 public:
  void notify_one() {
    ::WakeConditionVariable(&_cv);
  }

  void notify_all() {
    ::WakeAllConditionVariable(&_cv);
  }

  void wait(Mutex& mtx) {
    ::SleepConditionVariableCS(&_cv, &mtx._cs, INFINITE);
  }

  auto wait_timeout_ms(Mutex& mtx, DWORD millis) -> bool {
    const auto ret = ::SleepConditionVariableCS(&_cv, &mtx._cs, millis);
    return ret != 0;
  }
};

}  // namespace sfc::sys::sync
