#include "rc.inl"

#include "rc/sync/mutex.h"

namespace rc::sync {

pub Mutex::Mutex() = default;
pub Mutex::~Mutex() = default;
pub Mutex::Mutex(Mutex&&) noexcept = default;

pub auto Mutex::lock() -> Guard {
  _inner.lock();
  return Guard{*this};
}

pub Mutex::Guard::Guard(Mutex& mtx) noexcept : _mutex(&mtx) {}

pub Mutex::Guard::~Guard() {
  if (_mutex == nullptr) return;
  _mutex->_inner.unlock();
}

pub Mutex::Guard::Guard(Guard&& other) noexcept : _mutex{other._mutex} {
  other._mutex = nullptr;
}

}  // namespace rc::sync
