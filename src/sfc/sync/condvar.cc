#include "sfc/sync/condvar.h"

#include "sfc/sys/thread.h"

namespace sfc::sync {

namespace sys_imp = sys::thread;

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

  _inn->wait(*lock._mtx->_inn);
}

auto Condvar::wait_timeout(LockGuard& lock, const time::Duration& dur) -> bool {
  if (!_inn || !lock._mtx) {
    return false;
  }

  return _inn->wait_timeout_ms(*lock._mtx->_inn, dur.as_millis());
}

}  // namespace sfc::sync
