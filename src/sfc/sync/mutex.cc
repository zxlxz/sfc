#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/sync.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/sync.inl"
#endif

#include "sfc/sync/mutex.h"
#include "sfc/thread.h"

namespace sfc::sync {

Mutex::Mutex() noexcept : _inn{} {}

Mutex::~Mutex() noexcept {}

Mutex::Mutex(Mutex&& other) noexcept : _inn{mem::move(other._inn)} {}

Mutex& Mutex::operator=(Mutex&& other) noexcept = default;

auto Mutex::lock() noexcept -> Guard {
  _inn.lock();
  return Guard{*this};
}

auto Mutex::try_lock() noexcept -> Option<Guard> {
  if (!_inn.try_lock()) {
    return {};
  }
  return Guard{*this};
}

Mutex::Guard::Guard(Mutex& lock, passkey_t) : _lock{&lock} {}

Mutex::Guard::~Guard() noexcept {
  if (_lock == nullptr) return;
  _lock->_inn.unlock();
}

auto Mutex::Guard::inner() -> sys::Mutex& {
  return _lock->_inn;
}

ReentrantLock::ReentrantLock() noexcept : _mutex{}, _owner{0}, _count{0} {}

ReentrantLock::~ReentrantLock() noexcept {}

ReentrantLock::ReentrantLock(ReentrantLock&&) noexcept = default;

ReentrantLock& ReentrantLock::operator=(ReentrantLock&&) noexcept = default;

auto ReentrantLock::Tid::contains(u32 tid) const -> bool {
  return _id.load(Ordering::Acquire) == tid;
}

void ReentrantLock::Tid::set(u32 tid) {
  _id.store(tid, Ordering::Release);
}

auto ReentrantLock::lock() noexcept -> Guard {
  const auto this_id = thread::current_id();

  if (_owner.contains(this_id)) {
    _count += 1;
  } else {
    _mutex.lock();
    _owner.set(this_id);
    _count = 1;
  }
  return Guard{*this};
}

auto ReentrantLock::try_lock() noexcept -> Option<Guard> {
  const auto this_id = thread::current_id();

  if (_owner.contains(this_id)) {
    _count += 1;
    return Guard{*this};
  }

  if (_mutex.try_lock()) {
    _owner.set(this_id);
    _count = 1;
    return Guard{*this};
  }

  return {};
}

ReentrantLock::Guard::Guard(ReentrantLock& lock, passkey_t) : _lock{&lock} {}

ReentrantLock::Guard::~Guard() noexcept {
  if (_lock == nullptr) return;

  // Safety: we own the lock
  auto& lock = *_lock;
  lock._count -= 1;
  if (lock._count == 0) {
    lock._owner.set(0);
    lock._mutex.unlock();
  }
}

}  // namespace sfc::sync
