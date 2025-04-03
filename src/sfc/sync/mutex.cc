#include "mutex.h"

#include "sfc/sys/sync.inl"

namespace sfc::sync {

namespace sys_imp = sys::sync;

struct Mutex::Inn : sys_imp::Mutex {};

Mutex::Mutex() : _inn{Box<Inn>::xnew()} {}

Mutex::~Mutex() {}

Mutex::Mutex(Mutex&&) noexcept = default;

Mutex& Mutex::operator=(Mutex&&) noexcept = default;

auto Mutex::lock() -> LockGuard {
  return LockGuard{*this};
}

LockGuard::LockGuard(Mutex& mtx) : _mtx{&mtx} {
  if (mtx._inn) {
    mtx._inn->lock();
  }
}

LockGuard::~LockGuard() {
  this->unlock();
}

LockGuard::LockGuard(LockGuard&&) noexcept = default;

LockGuard& LockGuard::operator=(LockGuard&&) noexcept = default;

void LockGuard::unlock() {
  if (!_mtx || !_mtx->_inn) {
    return;
  }
  _mtx->_inn->unlock();
}

}  // namespace sfc::sync
