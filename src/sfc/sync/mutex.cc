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

Mutex::Guard::Guard(Mutex& lock) : _lock{&lock} {}

Mutex::Guard::~Guard() noexcept {
  if (_lock == nullptr) return;
  _lock->_inn.unlock();
}

auto Mutex::Guard::inner() -> sys::Mutex& {
  return _lock->_inn;
}

void ReentrantLock::Inn::lock() {
  const auto current_id = thread::current_id();
  const auto ownner_id = _owner.load(Ordering::Acquire);
  if (ownner_id == current_id) {
    _count += 1;
    return;
  }

  _imp.lock();
  _owner.store(current_id, Ordering::Release);
  _count = 1;
}

bool ReentrantLock::Inn::try_lock() {
  const auto current_id = thread::current_id();
  const auto ownner_id = _owner.load(Ordering::Acquire);
  if (ownner_id == current_id) {
    _count += 1;
    return true;
  }

  if (!_imp.try_lock()) {
    return false;
  }

  _owner.store(current_id, Ordering::Release);
  _count = 1;
  return true;
}

void ReentrantLock::Inn::unlock() {
  const auto current_id = thread::current_id();
  const auto ownner_id = _owner.load(Ordering::Acquire);
  if (ownner_id != current_id) {
    return;
  }

  _count -= 1;
  if (_count == 0) {
    _owner.store(0, Ordering::Release);
    _imp.unlock();
  }
}

ReentrantLock::ReentrantLock() noexcept : _inn{} {}

ReentrantLock::~ReentrantLock() noexcept {}

ReentrantLock::ReentrantLock(ReentrantLock&&) noexcept = default;

ReentrantLock& ReentrantLock::operator=(ReentrantLock&&) noexcept = default;

auto ReentrantLock::lock() noexcept -> Guard {
  _inn.lock();
  return Guard{*this};
}

auto ReentrantLock::try_lock() noexcept -> Option<Guard> {
  const auto ret = _inn.try_lock();
  if (!ret) {
    return {};
  }
  return Guard{*this};
}

ReentrantLock::Guard::Guard(ReentrantLock& lock) : _lock{&lock} {}

ReentrantLock::Guard::~Guard() noexcept {
  if (_lock == nullptr) return;
  _lock->_inn.unlock();
}

}  // namespace sfc::sync
