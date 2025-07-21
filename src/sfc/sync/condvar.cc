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
#ifdef _WIN32
  Inn() {
    sys_imp::init(_raw);
  }

  ~Inn() {
    sys_imp::drop(_raw);
  }
#endif
};

Condvar::Condvar() : _inn{Box<Inn>::xnew()} {}

Condvar::~Condvar() {}

Condvar::Condvar(Condvar&&) noexcept = default;

Condvar& Condvar::operator=(Condvar&&) noexcept = default;

void Condvar::notify_one() {
  panicking::assert(_inn, "Condvar::notify_one: on a dropped object");
  sys_imp::notify_one(_inn->_raw);
}

void Condvar::notify_all() {
  panicking::assert(_inn, "Condvar::notify_all: on a dropped object");
  sys_imp::notify_all(_inn->_raw);
}

void Condvar::wait(Mutex::Guard& lock) {
  panicking::assert(_inn, "Condvar::wait: on a dropped object");
  sys_imp::wait(_inn->_raw, lock._mtx->_raw);
}

auto Condvar::wait_timeout(Mutex::Guard& lock, const time::Duration& dur) -> bool {
  panicking::assert(_inn, "Condvar::wait_timeout: on a dropped object");
  const auto ret = sys_imp::wait_timeout_ns(_inn->_raw, lock._mtx->_raw, dur.as_nanos());
  return ret;
}

}  // namespace sfc::sync
