#include "sfc/sync/mutex.h"
#include "sfc/sys/sync.h"
#include "sfc/thread.h"

namespace sfc::sync {

struct Tid {
  Atomic<u32> _0{0};

 public:
  auto contains(u32 owner) const -> bool {
    return owner == _0.load(Ordering::Relaxed);
  }

  void set(u32 owner) {
    return _0.store(owner, Ordering::Relaxed);
  }
};

struct Mutex::Inn : sys::Mutex {};

struct ReentrantLock::Inn : sys::Mutex {
  Tid _owner{0};
  u32 _count{0};
};

Mutex::Mutex() noexcept : _inn{new Inn{}} {
  _inn->init();
}

Mutex::~Mutex() noexcept {
  if (_inn == nullptr) {
    return;
  }

  _inn->destroy();
  delete _inn;
  _inn = nullptr;
}

Mutex::Mutex(Mutex&& other) noexcept : _inn{other._inn} {
  other._inn = nullptr;
}

Mutex& Mutex::operator=(Mutex&& other) noexcept {
  if (this != &other) {
    mem::swap(_inn, other._inn);
  }
  return *this;
}

auto Mutex::lock() const noexcept -> Guard {
  _inn->lock();
  return Guard{*_inn};
}

auto Mutex::try_lock() const noexcept -> Option<Guard> {
  if (!_inn->try_lock()) {
    return None{};
  }
  return Guard{*_inn};
}

Mutex::Guard::Guard(Inn& mtx) noexcept : _inn{&mtx} {}

Mutex::Guard::Guard(Guard&& other) noexcept : _inn{mem::take(other._inn)} {}

Mutex::Guard::~Guard() noexcept {
  if (_inn == nullptr) return;
  _inn->unlock();
}

auto Mutex::Guard::inner() const -> Inn& {
  return *_inn;
}

ReentrantLock::ReentrantLock() noexcept : _inn{new Inn{}} {
  _inn->init();
}

ReentrantLock::~ReentrantLock() noexcept {
  if (_inn == nullptr) {
    return;
  }

  _inn->destroy();
  delete _inn;
  _inn = nullptr;
}

ReentrantLock::ReentrantLock(ReentrantLock&& other) noexcept : _inn{mem::take(other._inn)} {}

ReentrantLock& ReentrantLock::operator=(ReentrantLock&& other) noexcept {
  if (this != &other) {
    mem::swap(_inn, other._inn);
  }
  return *this;
}

auto ReentrantLock::lock() const noexcept -> Guard {
  const auto current_id = thread::current_id();
  if (_inn->_owner.contains(current_id)) {
    _inn->_count += 1;
    return Guard{*_inn};
  }

  _inn->lock();
  _inn->_owner.set(current_id);
  _inn->_count = 1;
  return Guard{*_inn};
}

auto ReentrantLock::try_lock() const noexcept -> Option<Guard> {
  const auto current_id = thread::current_id();
  if (_inn->_owner.contains(current_id)) {
    _inn->_count += 1;
    return Guard{*_inn};
  }

  if (!_inn->try_lock()) {
    return None{};
  }

  _inn->_owner.set(current_id);
  _inn->_count = 1;
  return Guard{*_inn};
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
