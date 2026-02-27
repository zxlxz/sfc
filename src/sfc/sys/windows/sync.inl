#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::sync {

using tid_t = DWORD;
using mtx_t = SRWLOCK;
using cnd_t = CONDITION_VARIABLE;

inline auto get_tid() -> tid_t {
  return ::GetCurrentThreadId();
}

inline void mtx_init(SRWLOCK& mtx) {
  ::InitializeSRWLock(&mtx);
}

inline void mtx_destroy(SRWLOCK& mtx) {
  (void)mtx;
}

inline void mtx_lock(SRWLOCK& mtx) {
  ::AcquireSRWLockExclusive(&mtx);
}

inline void mtx_unlock(SRWLOCK& mtx) {
  ::ReleaseSRWLockExclusive(&mtx);
}

inline bool mtx_trylock(SRWLOCK& mtx) {
  return ::TryAcquireSRWLockExclusive(&mtx);
}

inline void cnd_init(CONDITION_VARIABLE& cond) {
  ::InitializeConditionVariable(&cond);
}

inline void cnd_destroy(CONDITION_VARIABLE& cond) {
  (void)cond;
}

inline void cnd_signal(CONDITION_VARIABLE& cond) {
  ::WakeConditionVariable(&cond);
}

inline void cnd_broadcast(CONDITION_VARIABLE& cond) {
  ::WakeAllConditionVariable(&cond);
}

inline void cnd_wait(CONDITION_VARIABLE& cond, SRWLOCK& mtx) {
  ::SleepConditionVariableSRW(&cond, &mtx, INFINITE, 0);
}

inline auto cnd_timedwait(CONDITION_VARIABLE& cond, SRWLOCK& mtx, DWORD millis) -> bool {
  return ::SleepConditionVariableSRW(&cond, &mtx, millis, 0);
}

}  // namespace sfc::sys::sync
