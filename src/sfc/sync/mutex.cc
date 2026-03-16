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
  return {option::Some{}, trait::passkey_t{*this}};
}

Mutex::Guard::Guard(trait::passkey_t<Mutex> lock) : _lock{lock._val} {}

Mutex::Guard::~Guard() noexcept {
  _lock._inn.unlock();
}

ReentrantLock::ReentrantLock() noexcept : _mutex{}, _owner{0}, _count{0} {}

ReentrantLock::~ReentrantLock() noexcept {}

ReentrantLock::ReentrantLock(ReentrantLock&&) noexcept = default;

ReentrantLock& ReentrantLock::operator=(ReentrantLock&&) noexcept = default;

auto ReentrantLock::lock() noexcept -> Guard {
  const auto thr = thread::current();

  if (_owner.load(Ordering::Acquire) == thr.id) {
    _count.fetch_add(1, sync::Ordering::Relaxed);
    return Guard{{*this}};
  }

  _mutex.lock();
  _owner.store(thr.id, sync::Ordering::Release);
  _count.store(1, sync::Ordering::Relaxed);
  return Guard{*this};
}

auto ReentrantLock::try_lock() noexcept -> Option<Guard> {
  const auto thr = thread::current();

  if (_owner.load(Ordering::Acquire) == thr.id) {
    _count.fetch_add(1, sync::Ordering::Relaxed);
    return {option::Some{}, trait::passkey_t{*this}};
  }

  if (!_mutex.try_lock()) {
    return {};
  }
  _owner.store(thr.id, sync::Ordering::Release);
  _count.store(1, sync::Ordering::Relaxed);
  return {option::Some{}, trait::passkey_t{*this}};
}

ReentrantLock::Guard::Guard(trait::passkey_t<ReentrantLock> lock) : _lock{lock._val} {}

ReentrantLock::Guard::~Guard() noexcept {
  const auto thr = thread::current();

  if (_lock._owner.load(sync::Ordering::Acquire) != thr.id) {
    return;
  }

  if (_lock._count.fetch_sub(1, sync::Ordering::AcqRel) == 1) {
    _lock._owner.store(0, sync::Ordering::Release);
    _lock._mutex.unlock();
  }
}

}  // namespace sfc::sync
