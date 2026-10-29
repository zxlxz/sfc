#include "sfc/sync/mutex.h"
#include "sfc/sync/atomic.h"
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

class Mutex::Inn : public sys::Mutex {};

class ReentrantLock::Inn {
  sys::Mutex _mutex;
  Tid _owner{0};
  u32 _lock_count{0};

 public:
  void lock() {
    const auto current_id = thread::current_id();
    if (_owner.contains(current_id)) {
      _lock_count += 1;
    } else {
      _mutex.lock();
      _owner.set(current_id);
      _lock_count = 1;
    }
  }

  bool try_lock() {
    const auto current_id = thread::current_id();
    if (_owner.contains(current_id)) {
      _lock_count += 1;
    } else if (_mutex.try_lock()) {
      _owner.set(current_id);
      _lock_count = 1;
    } else {
      return false;
    }
    return true;
  }

  void unlock() {
    _lock_count -= 1;
    if (_lock_count == 0) {
      _owner.set(0);
      _mutex.unlock();
    }
  }
};

Mutex::Mutex() noexcept : _inn{new Inn()} {}

Mutex::~Mutex() noexcept {
  if (_inn == nullptr) {
    return;
  }

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

ReentrantLock::ReentrantLock() noexcept : _inn{new Inn{}} {}

ReentrantLock::~ReentrantLock() noexcept {
  if (_inn == nullptr) {
    return;
  }

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
  _inn->lock();
  return Guard{*_inn};
}

auto ReentrantLock::try_lock() const noexcept -> Option<Guard> {
  const auto ret = _inn->try_lock();
  if (!ret) {
    return None{};
  }
  return Guard{*_inn};
}

ReentrantLock::Guard::Guard(Inn& inn) noexcept : _inn{&inn} {}

ReentrantLock::Guard::Guard(Guard&& other) noexcept : _inn{mem::take(other._inn)} {}

ReentrantLock::Guard::~Guard() noexcept {
  if (_inn == nullptr) {
    return;
  }

  _inn->unlock();
}

}  // namespace sfc::sync
