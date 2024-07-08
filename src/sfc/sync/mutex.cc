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
  if (_inn) {
    _inn.ptr()->lock();
  }
  return LockGuard{*this};
}

void Mutex::unlock() {
  if (!_inn) {
    return;
  }
  _inn.ptr()->unlock();
}

}  // namespace sfc::sync
