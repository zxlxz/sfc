#include "sfc/sync/mutex.h"
#include "sfc/thread.h"

namespace sfc::sync {

auto Tid::contains(u32 owner) const -> bool {
  return owner == _0.load(Ordering::Relaxed);
}

void Tid::set(u32 owner) {
  _0.store(owner, Ordering::Relaxed);
}

Mutex::Mutex() noexcept : _inn{sys::Mutex{}} {}

Mutex::~Mutex() noexcept {}

Mutex::Mutex(Mutex&& other) noexcept : _inn{mem::move(other._inn)} {}

Mutex& Mutex::operator=(Mutex&& other) noexcept = default;

auto Mutex::lock() const noexcept -> Guard {
  _inn.lock();
  return Guard{_inn};
}

auto Mutex::try_lock() const noexcept -> Option<Guard> {
  if (!_inn.try_lock()) {
    return None{};
  }
  return Guard{_inn};
}

Mutex::Guard::Guard(Inn& mtx) noexcept : _inn{&mtx} {}
Mutex::Guard::Guard(Guard&& other) noexcept : _inn{mem::take(other._inn)} {}

Mutex::Guard::~Guard() noexcept {
  if (_inn == nullptr) return;
  _inn->unlock();
}

auto Mutex::Guard::inner() const -> sys::Mutex& {
  return *_inn;
}

ReentrantLock::ReentrantLock() noexcept : _inn{sys::Mutex{}} {}
ReentrantLock::~ReentrantLock() noexcept {}

ReentrantLock::ReentrantLock(ReentrantLock&&) noexcept = default;
ReentrantLock& ReentrantLock::operator=(ReentrantLock&&) noexcept = default;

auto ReentrantLock::lock() const noexcept -> Guard {
  const auto current_id = thread::current_id();
  if (_inn._owner.contains(current_id)) {
    _inn._count += 1;
    return Guard{_inn};
  }

  _inn.lock();
  _inn._owner.set(current_id);
  _inn._count = 1;
  return Guard{_inn};
}

auto ReentrantLock::try_lock() const noexcept -> Option<Guard> {
  const auto current_id = thread::current_id();
  if (_inn._owner.contains(current_id)) {
    _inn._count += 1;
    return Guard{_inn};
  }

  if (!_inn.try_lock()) {
    return None{};
  }

  _inn._owner.set(current_id);
  _inn._count = 1;
  return Guard{_inn};
}

ReentrantLock::Guard::Guard(Inn& inn) noexcept : _inn{&inn} {}
ReentrantLock::Guard::Guard(Guard&& other) noexcept : _inn{mem::take(other._inn)} {}

ReentrantLock::Guard::~Guard() noexcept {
  if (_inn == nullptr) {
    return;
  }

  _inn->_count -= 1;
  if (_inn->_count == 0) {
    _inn->_owner.set(0);
    _inn->unlock();
  }
}

}  // namespace sfc::sync
