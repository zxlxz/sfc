#include "sfc/sync/mutex.h"
#include "sfc/thread.h"

namespace sfc::sync {

Mutex::Mutex() noexcept : _inn{} {}

Mutex::~Mutex() noexcept {}

Mutex::Mutex(Mutex&& other) noexcept : _inn{mem::move(other._inn)} {}

Mutex& Mutex::operator=(Mutex&& other) noexcept = default;

auto Mutex::lock() noexcept -> Guard {
  _inn.lock();

  auto res = Guard{};
  res._lock = this;
  return res;
}

auto Mutex::try_lock() noexcept -> Option<Guard> {
  if (!_inn.try_lock()) {
    return {};
  }

  auto res = Guard{};
  res._lock = this;
  return res;
}

Mutex::Guard::Guard() noexcept : _lock{nullptr} {}

Mutex::Guard::~Guard() noexcept {
  if (_lock == nullptr) return;
  _lock->_inn.unlock();
}

Mutex::Guard::Guard(Guard&& other) noexcept : _lock{other._lock} {
  other._lock = nullptr;
}

Mutex::Guard& Mutex::Guard::operator=(Guard&& other) noexcept {
  if (this != &other) {
    mem::swap(_lock, other._lock);
  }
  return *this;
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

  auto res = Guard{};
  res._lock = this;
  return res;
}

auto ReentrantLock::try_lock() noexcept -> Option<Guard> {
  const auto ret = _inn.try_lock();
  if (!ret) {
    return {};
  }

  auto res = Guard{};
  res._lock = this;
  return res;
}

ReentrantLock::Guard::Guard() noexcept : _lock{nullptr} {}

ReentrantLock::Guard::~Guard() noexcept {
  if (_lock == nullptr) return;
  _lock->_inn.unlock();
}

ReentrantLock::Guard::Guard(Guard&& other) noexcept : _lock{other._lock} {
  other._lock = nullptr;
}

ReentrantLock::Guard& ReentrantLock::Guard::operator=(Guard&& other) noexcept {
  if (this != &other) {
    mem::swap(_lock, other._lock);
  }
  return *this;
}

}  // namespace sfc::sync
