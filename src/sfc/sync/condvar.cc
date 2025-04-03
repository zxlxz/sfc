#include "condvar.h"

#include "sfc/sys/sync.inl"

namespace sfc::sync {

namespace sys_imp = sys::sync;

struct Mutex::Inn : sys_imp::Mutex {};

struct Condvar::Inn : sys_imp::Condvar {};

Condvar::Condvar() : _inn{Box<Inn>::xnew()} {}

Condvar::~Condvar() {}

Condvar::Condvar(Condvar&&) noexcept = default;

void Condvar::notify_one() {
  if (!_inn) {
    return;
  }
  _inn->notify_one();
}

void Condvar::notify_all() {
  if (!_inn) {
    return;
  }
  _inn->notify_all();
}

void Condvar::wait(LockGuard& lock) {
  if (!_inn || !lock._mtx) {
    return;
  }

  auto& mtx = *lock._mtx->_inn;
  _inn->wait(mtx);
}

auto Condvar::wait_timeout(LockGuard& lock, const time::Duration& dur) -> bool {
  if (!_inn || !lock._mtx) {
    return false;
  }

  auto& mtx = *lock._mtx->_inn;
  return _inn->wait_timeout_ms(mtx, dur.as_millis());
}

}  // namespace sfc::sync
