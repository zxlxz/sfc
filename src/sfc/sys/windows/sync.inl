#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::sync {

using tid_t = DWORD;
using mtx_t = SRWLOCK;
using cnd_t = CONDITION_VARIABLE;

inline auto get_tid() -> tid_t {
  return ::GetCurrentThreadId();
}

inline void mtx_init(mtx_t& mtx) {
  ::InitializeSRWLock(&mtx);
}

inline void mtx_destroy(mtx_t& mtx) {
  (void)mtx;
}

inline void mtx_lock(mtx_t& mtx) {
  ::AcquireSRWLockExclusive(&mtx);
}

inline void mtx_unlock(mtx_t& mtx) {
  ::ReleaseSRWLockExclusive(&mtx);
}

inline bool mtx_trylock(mtx_t& mtx) {
  return ::TryAcquireSRWLockExclusive(&mtx);
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
  ::SleepConditionVariableSRW(&cond, &mtx, INFINITE, 0);
}

inline auto cnd_timedwait(cnd_t& cond, mtx_t& mtx, DWORD millis) -> bool {
  return ::SleepConditionVariableSRW(&cond, &mtx, millis, 0);
}

}  // namespace sfc::sys::sync
