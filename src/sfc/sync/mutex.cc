#include "mutex.h"

#include "sfc/sys/thread.h"

namespace sfc::sync {

namespace sys_imp = sys::thread;

struct Mutex::Inn : sys_imp::Mutex {};

Mutex::Mutex() : _inn{Box<Inn>::xnew()} {}

Mutex::~Mutex() {}

Mutex::Mutex(Mutex&&) noexcept = default;

Mutex& Mutex::operator=(Mutex&&) noexcept = default;

auto Mutex::lock() -> LockGuard {
  _inn->lock();
  return LockGuard{*this};
}

LockGuard::LockGuard(Mutex& mtx) : _mtx{&mtx} {
  if (mtx._inn) {
    mtx._inn->lock();
  }
}

LockGuard::~LockGuard() {
  if (!_mtx || !_mtx->_inn) {
    return;
  }
  _mtx->_inn->unlock();
}

}  // namespace sfc::sync
