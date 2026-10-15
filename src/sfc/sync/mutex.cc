#include "sfc/sync/mutex.h"
#include "sfc/thread.h"

namespace sfc::sync {

Mutex::Mutex() noexcept : _inn{sys::Mutex{}} {}

Mutex::~Mutex() noexcept {}

Mutex::Mutex(Mutex&& other) noexcept : _inn{mem::move(other._inn)} {}

Mutex& Mutex::operator=(Mutex&& other) noexcept = default;

auto Mutex::lock() noexcept -> Guard {
  _inn.lock();
  return Guard{_inn};
}

auto Mutex::try_lock() noexcept -> Option<Guard> {
  if (!_inn.try_lock()) {
    return None{};
  }
  return Guard{_inn};
}

Mutex::Guard::Guard(Inn& mtx) noexcept : _inn{&mtx} {}

Mutex::Guard::~Guard() noexcept {
  if (_inn == nullptr) return;
  _inn->unlock();
}

Mutex::Guard::Guard(Guard&& other) noexcept : _inn{mem::take(other._inn)} {}

Mutex::Guard& Mutex::Guard::operator=(Guard&& other) noexcept {
  if (this != &other) {
    mem::swap(_inn, other._inn);
  }
  return *this;
}

auto Mutex::Guard::inner() -> sys::Mutex& {
  return *_inn;
}

ReentrantLock::ReentrantLock() noexcept : _inn{sys::Mutex{}} {}

ReentrantLock::~ReentrantLock() noexcept {}

ReentrantLock::ReentrantLock(ReentrantLock&&) noexcept = default;

ReentrantLock& ReentrantLock::operator=(ReentrantLock&&) noexcept = default;

auto ReentrantLock::lock() noexcept -> Guard {
  const auto current_id = thread::current_id();
  const auto ownner_id = _inn._owner.load(Ordering::Relaxed);
  if (ownner_id == current_id) {
    _inn._count += 1;
    return Guard{_inn};
  }

  _inn.lock();
  _inn._owner.store(current_id, Ordering::Relaxed);
  _inn._count = 1;
  return Guard{_inn};
}

auto ReentrantLock::try_lock() noexcept -> Option<Guard> {
  const auto current_id = thread::current_id();
  const auto ownner_id = _inn._owner.load(Ordering::Relaxed);
  if (ownner_id == current_id) {
    _inn._count += 1;
    return Guard{_inn};
  }

  if (!_inn.try_lock()) {
    return None{};
  }

  _inn._owner.store(current_id, Ordering::Relaxed);
  _inn._count = 1;
  return Guard{_inn};
}

ReentrantLock::Guard::Guard(Inn& inn) noexcept : _inn{&inn} {}

ReentrantLock::Guard::~Guard() noexcept {
  if (_inn == nullptr) {
    return;
  }

  const auto current_id = thread::current_id();
  const auto ownner_id = _inn->_owner.load(Ordering::Relaxed);
  if (ownner_id != current_id) {
    return;
  }

  _inn->_count -= 1;
  if (_inn->_count == 0) {
    _inn->_owner.store(0, Ordering::Relaxed);
    _inn->unlock();
  }
}

ReentrantLock::Guard::Guard(Guard&& other) noexcept : _inn{mem::take(other._inn)} {}

ReentrantLock::Guard& ReentrantLock::Guard::operator=(Guard&& other) noexcept {
  if (this != &other) {
    mem::swap(_inn, other._inn);
  }
  return *this;
}

}  // namespace sfc::sync
