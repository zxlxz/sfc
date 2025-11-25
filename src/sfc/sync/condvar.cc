#include "sfc/sync/condvar.h"

#include "sfc/sys/sync.h"

namespace sfc::sync {

namespace sys_imp = sys::sync;

struct Mutex::Inn {
  sys_imp::mutex_t _raw;
};

struct Condvar::Inn {
  sys_imp::cond_t _raw;

 public:
  Inn() {
    sys_imp::init(_raw);
  }

  ~Inn() noexcept{
    sys_imp::drop(_raw);
  }

  Inn(const Inn&) = delete;
  Inn& operator=(const Inn&) = delete;
};

Condvar::Condvar() : _inn{Box<Inn>::xnew()} {}

Condvar::~Condvar() noexcept {}

Condvar::Condvar(Condvar&&) noexcept = default;

Condvar& Condvar::operator=(Condvar&&) noexcept = default;

void Condvar::notify_one() {
  if (!_inn) {
    return;
  }

  sys_imp::notify_one(_inn->_raw);
}

void Condvar::notify_all() {
  if (!_inn) {
    return;
  }

  sys_imp::notify_all(_inn->_raw);
}

auto Condvar::wait(Mutex::Guard& lock) -> bool {
  if (!_inn) {
    return false;
  }

  sys_imp::wait(_inn->_raw, lock._mtx->_raw);
  return true;
}

auto Condvar::wait_timeout(Mutex::Guard& lock, time::Duration dur) -> bool {
  if (!_inn) {
    return false;
  }

  const auto millis = static_cast<u32>(dur.as_millis());
  const auto ret = sys_imp::wait_timeout_ms(_inn->_raw, lock._mtx->_raw, millis);
  return ret;
}

}  // namespace sfc::sync
