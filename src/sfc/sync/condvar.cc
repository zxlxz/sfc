#include "condvar.h"

#include <pthread.h>

#include "sfc/time.h"

namespace sfc::sync {

struct Mutex::Inn {
  pthread_mutex_t _raw;
};

struct Condvar::Inn {
  pthread_cond_t _raw = PTHREAD_COND_INITIALIZER;

  void notify_one() {
    ::pthread_cond_signal(&_raw);
  }

  void notify_all() {
    ::pthread_cond_broadcast(&_raw);
  }

  void wait(Mutex::Inn& mtx) {
    ::pthread_cond_wait(&_raw, &mtx._raw);
  }

  auto wait_timeout(Mutex::Inn& mtx, time::Duration dur) -> bool {
    struct timespec ts {};
    ::clock_gettime(CLOCK_REALTIME, &ts);

    ts.tv_sec += static_cast<i64>(dur.as_secs());
    ts.tv_nsec += static_cast<i64>(dur.subsec_nanos());

    const auto ret = ::pthread_cond_timedwait(&_raw, &mtx._raw, &ts);
    return ret == 0;
  }
};

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
  if (!_inn && !lock._mtx) {
    return;
  }

  auto& mtx = *lock._mtx->_inn;
  _inn->wait(mtx);
}

auto Condvar::wait_timeout(LockGuard& lock, time::Duration dur) -> bool {
  if (!_inn && !lock._mtx) {
    return false;
  }

  auto& mtx = *lock._mtx->_inn;
  return _inn->wait_timeout(mtx, dur);
}

auto Condvar::wait_timeout_ms(LockGuard& lock, u32 ms) -> bool {
  const auto dur = time::Duration::from_millis(ms);
  return this->wait_timeout(lock, dur);
}

}  // namespace sfc::sync
