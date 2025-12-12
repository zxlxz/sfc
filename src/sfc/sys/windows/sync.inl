#pragma once
#ifdef _WIN32
#include <Windows.h>

namespace sfc::sys::sync {

using tid_t = DWORD;
using mtx_t = CRITICAL_SECTION;
using cnd_t = CONDITION_VARIABLE;

inline auto get_tid() -> tid_t {
  return ::GetCurrentThreadId();
}

inline void mtx_init(mtx_t& mtx) {
  ::InitializeCriticalSection(&mtx);
}

inline void mtx_drop(mtx_t& mtx) {
  ::DeleteCriticalSection(&mtx);
}

inline void mtx_lock(mtx_t& mtx) {
  ::EnterCriticalSection(&mtx);
}

inline void mtx_unlock(mtx_t& mtx) {
  ::LeaveCriticalSection(&mtx);
}

inline void cond_init(cnd_t& cond) {
  ::InitializeConditionVariable(&cond);
}

inline void cond_drop(cnd_t& cond) {
  (void)cond;
}

inline void cond_notify_one(cnd_t& cond) {
  ::WakeConditionVariable(&cond);
}

inline void cond_notify_all(cnd_t& cond) {
  ::WakeAllConditionVariable(&cond);
}

inline void cond_wait(cnd_t& cond, mtx_t& mtx) {
  ::SleepConditionVariableCS(&cond, &mtx, INFINITE);
}

inline auto cond_wait_timeout_ms(cnd_t& cond, mtx_t& mtx, DWORD millis) -> bool {
  return ::SleepConditionVariableCS(&cond, &mtx, millis);
}

}  // namespace sfc::sys::sync
#endif
