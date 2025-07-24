#pragma once

#include <Windows.h>

namespace sfc::sys::sync {

struct CRITICAL_SECTION_EX : CRITICAL_SECTION {
  CRITICAL_SECTION_EX() {
    InitializeCriticalSection(this);
  }

  ~CRITICAL_SECTION_EX() {
    DeleteCriticalSection(this);
  }
};

using mutex_t = CRITICAL_SECTION_EX;
using cond_t = CONDITION_VARIABLE;

inline void lock(mutex_t& mtx) {
  ::EnterCriticalSection(&mtx);
}

inline void unlock(mutex_t& mtx) {
  ::LeaveCriticalSection(&mtx);
}

inline void init(cond_t& cond) {
  InitializeConditionVariable(&cond);
}

inline void drop(cond_t& cond) {}

inline void notify_one(cond_t& cond) {
  ::WakeConditionVariable(&cond);
}

inline void notify_all(cond_t& cond) {
  ::WakeAllConditionVariable(&cond);
}

inline void wait(cond_t& cond, mutex_t& mtx) {
  ::SleepConditionVariableCS(&cond, &mtx, INFINITE);
}

inline auto wait_timeout_ns(cond_t& cond, mutex_t& mtx, DWORD64 nanos) -> bool {
  static constexpr auto NANOS_PER_MILLI = 1000000U;

  const auto millis = static_cast<DWORD>(nanos / NANOS_PER_MILLI);
  return ::SleepConditionVariableCS(&cond, &mtx, millis);
}

}  // namespace sfc::sys::sync
