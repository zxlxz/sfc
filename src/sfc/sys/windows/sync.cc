#include <Windows.h>
#undef min
#undef max

#include "sfc/sys/windows/sync.h"

namespace sfc::sys::windows {

struct Mutex::Inn {
  SRWLOCK _0;
};

Mutex::Mutex() {
  _ptr = new Inn{};
  ::InitializeSRWLock(&_ptr->_0);
}

Mutex::~Mutex() {
  if (_ptr == nullptr) return;
  delete _ptr;
}

Mutex::Mutex(Mutex&& other) noexcept : _ptr{other._ptr} {
  other._ptr = nullptr;
}

Mutex& Mutex::operator=(Mutex&& other) noexcept {
  if (this != &other) {
    mem::swap(_ptr, other._ptr);
  }
  return *this;
}

void* Mutex::raw() {
  return &_ptr->_0;
}

void Mutex::lock() {
  ::AcquireSRWLockExclusive(&_ptr->_0);
}

void Mutex::unlock() {
  ::ReleaseSRWLockExclusive(&_ptr->_0);
}

auto Mutex::try_lock() -> bool {
  const auto ret = ::TryAcquireSRWLockExclusive(&_ptr->_0);
  return bool(ret);
}

struct Condvar::Inn {
  CONDITION_VARIABLE _0;
};

Condvar::Condvar() {
  _ptr = new Inn{};
  ::InitializeConditionVariable(&_ptr->_0);
}

Condvar::~Condvar() {
  if (_ptr == nullptr) return;
  delete _ptr;
}

Condvar::Condvar(Condvar&& other) noexcept : _ptr{other._ptr} {
  other._ptr = nullptr;
}

Condvar& Condvar::operator=(Condvar&& other) noexcept {
  if (this != &other) {
    mem::swap(_ptr, other._ptr);
  }
  return *this;
}

void Condvar::notify_one() {
  ::WakeConditionVariable(&_ptr->_0);
}

void Condvar::notify_all() {
  ::WakeAllConditionVariable(&_ptr->_0);
}

void Condvar::wait(Mutex& mtx) {
  (void)::SleepConditionVariableSRW(&_ptr->_0, (SRWLOCK*)mtx.raw(), INFINITE, 0);
}

auto Condvar::wait_timeout(Mutex& mtx, time::Duration dur) -> bool {
  const auto ms = num::saturating_cast<u32>(dur.as_millis());
  const auto ret = ::SleepConditionVariableSRW(&_ptr->_0, (SRWLOCK*)mtx.raw(), ms, 0);
  return bool(ret);
}

}  // namespace sfc::sys::windows
