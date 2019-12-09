#include "rc/sys/unix.inl"

#include "rc/io.h"

namespace rc::sys::unix::sync {

#pragma region mutex

Mutex::Mutex() {
  _raw = alloc::alloc<pthread_mutex_t>(1);
  ptr::write(_raw, pthread_mutex_t(PTHREAD_MUTEX_INITIALIZER));
}

Mutex::~Mutex() {
  if (_raw == nullptr) return;
  alloc::dealloc(_raw, 1);
}

Mutex::Mutex(Mutex&& other) noexcept : _raw(other._raw) {
  other._raw = nullptr;
}

auto Mutex::lock() -> void {
  const auto eid = ::pthread_mutex_lock(_raw);
  if (eid != 0) {
    throw io::Error::from_raw_os_error(eid);
  }
}

auto Mutex::unlock() -> void {
  const auto eid = pthread_mutex_unlock(_raw);
  if (eid != 0) {
    throw io::Error::from_raw_os_error(eid);
  }
}

auto Mutex::trylock() -> bool {
  const auto eid = ::pthread_mutex_trylock(_raw);
  if (eid != 0) {
    if (eid == EBUSY) return false;
    throw io::Error::from_raw_os_error(eid);
  }
  return true;
}

#pragma endregion

#pragma region condvar

CondVar::CondVar() {
  _raw = alloc::alloc<pthread_cond_t>(1);
  ptr::write(_raw, pthread_cond_t(PTHREAD_COND_INITIALIZER));
}

CondVar::~CondVar() {
  if (_raw == nullptr) return;
  alloc::dealloc(_raw, 1);
  _raw = nullptr;
}

CondVar::CondVar(CondVar&& other) noexcept : _raw{other._raw} {
  other._raw = nullptr;
}

auto CondVar::wait(Mutex& mtx) -> void {
  const auto eid = ::pthread_cond_wait(_raw, mtx._raw);
  if (eid != 0) {
    throw io::Error::from_raw_os_error(eid);
  }
}

auto CondVar::wait_timeout(Mutex& mtx, time::Duration dur) -> bool {
  const auto ts = unix::dur2ts(dur);
  const auto eid = ::pthread_cond_timedwait(_raw, mtx._raw, &ts);
  if (eid != 0) {
    if (eid == ETIMEDOUT) return false;
    throw io::Error::from_raw_os_error(eid);
  }
  return true;
}

auto CondVar::notify_one() -> void {
  const auto eid = ::pthread_cond_signal(_raw);
  if (eid != 0) {
    throw io::Error::from_raw_os_error(eid);
  }
}

auto CondVar::notify_all() -> void {
  const auto eid = ::pthread_cond_broadcast(_raw);
  if (eid != 0) {
    throw io::Error::from_raw_os_error(eid);
  }
}

#pragma endregion

}  // namespace rc::sys::unix::sync
