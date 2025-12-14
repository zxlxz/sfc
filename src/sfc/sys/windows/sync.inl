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

inline void mtx_destroy(mtx_t& mtx) {
  ::DeleteCriticalSection(&mtx);
}

inline void mtx_lock(mtx_t& mtx) {
  ::EnterCriticalSection(&mtx);
}

inline void mtx_unlock(mtx_t& mtx) {
  ::LeaveCriticalSection(&mtx);
}

inline void cnd_init(cnd_t& cond) {
  ::InitializeConditionVariable(&cond);
}

inline void cnd_destroy(cnd_t& cond) {
  (void)cond;
}

inline void cnd_signal(cnd_t& cond) {
  ::WakeConditionVariable(&cond);
}

inline void cnd_broadcast(cnd_t& cond) {
  ::WakeAllConditionVariable(&cond);
}

inline void cnd_wait(cnd_t& cond, mtx_t& mtx) {
  ::SleepConditionVariableCS(&cond, &mtx, INFINITE);
}

inline auto cnd_timedwait(cnd_t& cond, mtx_t& mtx, DWORD millis) -> bool {
  return ::SleepConditionVariableCS(&cond, &mtx, millis);
}

}  // namespace sfc::sys::sync
#endif
