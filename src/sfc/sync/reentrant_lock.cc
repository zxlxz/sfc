#include "reentrant_lock.h"

#include "sfc/sys/thread.h"

namespace sfc::sync {

namespace sys_imp = sys::thread;

struct ReentrantLock::Inn : sys_imp::Mutex {
  Inn() : sys_imp::Mutex{true} {}
};

ReentrantLock::ReentrantLock() : _inn{Box<Inn>::xnew()} {}

ReentrantLock::~ReentrantLock() {}

ReentrantLock::ReentrantLock(ReentrantLock&&) noexcept = default;

ReentrantLock& ReentrantLock::operator=(ReentrantLock&&) noexcept = default;

auto ReentrantLock::lock() -> ReentrantLockGuard {
  _inn ? _inn->lock() : (void)0;
  return ReentrantLockGuard{*this};
}

ReentrantLockGuard::ReentrantLockGuard(ReentrantLock& mtx) : _mtx{&mtx} {}

ReentrantLockGuard::~ReentrantLockGuard() noexcept {
  if (!_mtx || !_mtx->_inn) {
    return;
  }
  _mtx->_inn->unlock();
}

}  // namespace sfc::sync