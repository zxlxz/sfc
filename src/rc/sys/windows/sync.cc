#include "rc/sys/windows.inl"

#include "rc/io.h"

namespace rc::sys::windows::sync {

#pragma region Mutex

Mutex::Mutex() {
  _raw = alloc::alloc<SRWLOCK>(1);
  ptr::write(_raw, SRWLOCK(SRWLOCK_INIT));
}

Mutex::~Mutex() {
  if (_raw == nullptr) return;
  alloc::dealloc(_raw, 1);
}

Mutex::Mutex(Mutex&& other) noexcept : _raw(other._raw) {
  other._raw = nullptr;
}

auto Mutex::lock() -> void {
  if (_raw == nullptr) return;
  ::AcquireSRWLockExclusive(_raw);
}

#pragma warning(push)
#pragma warning(disable: 26110)
auto Mutex::unlock() -> void {
  if (_raw == nullptr) return;
  ::ReleaseSRWLockExclusive(_raw);
}
#pragma warning(pop)

auto Mutex::trylock() -> bool {
  if (_raw == nullptr) return false;
  const auto r = ::TryAcquireSRWLockExclusive(_raw);
  return r != 0;
}
#pragma endregion

#pragma region CondVar

CondVar::CondVar() {
  _raw = alloc::alloc<CONDITION_VARIABLE>(1);
  ptr::write(_raw, CONDITION_VARIABLE(CONDITION_VARIABLE_INIT));
}

CondVar::~CondVar() {
  if (_raw == nullptr) return;
  alloc::dealloc(_raw, 1);
}

CondVar::CondVar(CondVar&& other) noexcept : _raw{other._raw} {
  other._raw = nullptr;
}

auto CondVar::wait(Mutex& mtx) -> void {
  if (_raw == nullptr) return;
  if (mtx._raw == nullptr) return;

  const auto res = ::SleepConditionVariableSRW(_raw, mtx._raw, INFINITE, 0);
  if (res == FALSE) {
    throw io::Error::last_os_error();
  }
}

auto CondVar::wait_timeout(Mutex& mtx, Duration dur) -> bool {
  const auto tms = sys::dur2timeout(dur);
  const auto res = ::SleepConditionVariableSRW(_raw, mtx._raw, tms, 0);

  if (res == FALSE) {
    const auto eid = os::last_error();
    if (eid != ERROR_TIMEOUT) {
      throw io::Error::from_raw_os_error(eid);
    }
    return false;
  }
  return true;
}

auto CondVar::notify_one() -> void {
  if (_raw == nullptr) return;
  ::WakeConditionVariable(_raw);
}

auto CondVar::notify_all() -> void {
  if (_raw == nullptr) return;
  ::WakeAllConditionVariable(_raw);
}
#pragma endregion

}  // namespace rc::sys::windows::sync
