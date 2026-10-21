#include <Windows.h>

#include "sfc/sys/windows/sync.h"

namespace sfc::sys::windows {

Mutex::Mutex() {
  ::InitializeSRWLock(&_raw);
}

Mutex::~Mutex() {}

void Mutex::lock() {
  ::AcquireSRWLockExclusive(&_raw);
}

void Mutex::unlock() {
  ::ReleaseSRWLockExclusive(&_raw);
}

auto Mutex::try_lock() -> bool {
  const auto ret = ::TryAcquireSRWLockExclusive(&_raw);
  return bool(ret);
}

Condvar::Condvar() {
  ::InitializeConditionVariable(&_raw);
}

Condvar::~Condvar() {}

void Condvar::notify_one() {
  ::WakeConditionVariable(&_raw);
}

void Condvar::notify_all() {
  ::WakeAllConditionVariable(&_raw);
}

void Condvar::wait(Mutex& mtx) {
  (void)::SleepConditionVariableSRW(&_raw, &mtx._raw, INFINITE, 0);
}

auto Condvar::wait_timeout(Mutex& mtx, time::Duration dur) -> bool {
  const auto ms_u64 = dur.as_millis();
  const auto ms_u32 = ms_u64 < INFINITE ? u32(ms_u64) : INFINITE;

  const auto ret = ::SleepConditionVariableSRW(&_raw, &mtx._raw, ms_u32, 0);
  return bool(ret);
}

}  // namespace sfc::sys::windows
