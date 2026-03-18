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
  return {option::Some{}, *this};
}

Mutex::Guard::Guard(Mutex& lock, passkey_t) : _lock{lock} {}

Mutex::Guard::~Guard() noexcept {
  _lock._inn.unlock();
}

auto Mutex::Guard::inner() -> sys::Mutex& {
  return _lock._inn;
}

ReentrantLock::ReentrantLock() noexcept : _mutex{}, _owner{0}, _count{0} {}

ReentrantLock::~ReentrantLock() noexcept {}

ReentrantLock::ReentrantLock(ReentrantLock&&) noexcept = default;

ReentrantLock& ReentrantLock::operator=(ReentrantLock&&) noexcept = default;

auto ReentrantLock::lock() noexcept -> Guard {
  const auto thr = thread::current();

  if (_owner.load(Ordering::Acquire) == thr.id) {
    _count.fetch_add(1, Ordering::Relaxed);
    return Guard{{*this}};
  }

  _mutex.lock();
  _owner.store(thr.id, Ordering::Release);
  _count.store(1, Ordering::Relaxed);
  return Guard{*this};
}

auto ReentrantLock::try_lock() noexcept -> Option<Guard> {
  const auto thread_id = thread::current_id();

  if (_owner.load(Ordering::Acquire) == thread_id) {
    _count.fetch_add(1, Ordering::Relaxed);
    return {option::Some{}, *this};
  }

  if (!_mutex.try_lock()) {
    return {};
  }

  _owner.store(thread_id, Ordering::Release);
  _count.store(1, Ordering::Relaxed);
  return {option::Some{}, *this};
}

ReentrantLock::Guard::Guard(ReentrantLock& lock, passkey_t) : _lock{lock} {}

ReentrantLock::Guard::~Guard() noexcept {
  const auto thread_id = thread::current_id();

  if (_lock._owner.load(Ordering::Acquire) != thread_id) {
    return;
  }

  if (_lock._count.fetch_sub(1, Ordering::AcqRel) == 1) {
    _lock._owner.store(0, Ordering::Release);
    _lock._mutex.unlock();
  }
}

}  // namespace sfc::sync
