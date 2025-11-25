#pragma once
#ifdef _WIN32
#include <Windows.h>

namespace sfc::sys::sync {

using mutex_t = CRITICAL_SECTION;
using cond_t = CONDITION_VARIABLE;

inline void init(mutex_t& mtx) {
  ::InitializeCriticalSection(&mtx);
} 

inline void drop(mutex_t& mtx) {
  ::DeleteCriticalSection(&mtx);
}

inline void lock(mutex_t& mtx) {
  ::EnterCriticalSection(&mtx);
}

inline void unlock(mutex_t& mtx) {
  ::LeaveCriticalSection(&mtx);
}

inline void init(cond_t& cond) {
  ::InitializeConditionVariable(&cond);
}

inline void drop(cond_t& cond) {
  (void)cond;
}

inline void notify_one(cond_t& cond) {
  ::WakeConditionVariable(&cond);
}

inline void notify_all(cond_t& cond) {
  ::WakeAllConditionVariable(&cond);
}

inline void wait(cond_t& cond, mutex_t& mtx) {
  ::SleepConditionVariableCS(&cond, &mtx, INFINITE);
}

inline auto wait_timeout_ms(cond_t& cond, mutex_t& mtx, DWORD millis) -> bool {
  return ::SleepConditionVariableCS(&cond, &mtx, millis);
}

}  // namespace sfc::sys::sync
#endif
