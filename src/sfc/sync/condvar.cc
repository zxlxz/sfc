#include "sfc/sync/condvar.h"

#include "sfc/sys/sync.h"

namespace sfc::sync {

namespace sys_imp = sys::sync;

struct Mutex::Inn {
  sys_imp::mtx_t _raw;
};

struct Condvar::Inn {
  sys_imp::cnd_t _raw;

 public:
  Inn() noexcept {
    sys_imp::cond_init(_raw);
  }

  ~Inn() noexcept {
    sys_imp::cond_drop(_raw);
  }
};

Condvar::Condvar() noexcept: _inn{Box<Inn>::xnew()} {}

Condvar::~Condvar() noexcept {}

Condvar::Condvar(Condvar&&) noexcept = default;

Condvar& Condvar::operator=(Condvar&&) noexcept = default;

void Condvar::notify_one() noexcept{
  if (!_inn) {
    return;
  }

  sys_imp::cond_notify_one(_inn->_raw);
}

void Condvar::notify_all()noexcept{
  if (!_inn) {
    return;
  }

  sys_imp::cond_notify_all(_inn->_raw);
}

auto Condvar::wait(Mutex::Guard& lock) noexcept-> bool {
  if (!_inn) {
    return false;
  }

  sys_imp::cond_wait(_inn->_raw, lock._inn._raw);
  return true;
}

auto Condvar::wait_timeout(Mutex::Guard& lock, time::Duration dur) noexcept -> bool {
  if (!_inn) {
    return false;
  }

  const auto millis = static_cast<u32>(dur.as_millis());
  const auto ret = sys_imp::cond_wait_timeout_ms(_inn->_raw, lock._inn._raw, millis);
  return ret;
}

}  // namespace sfc::sync
