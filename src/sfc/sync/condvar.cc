#include "sfc/sys/sync.h"
#include "sfc/sync/condvar.h"

namespace sfc::sync {

class Mutex::Inn : public sys::Mutex {};

class Condvar::Inn : public sys::Condvar {
 public:
  using sys::Condvar::Condvar;
};

Condvar::Condvar() noexcept : _inn{Box<Inn>::new_()} {}
Condvar::~Condvar() noexcept = default;

Condvar::Condvar(Condvar&&) noexcept = default;
Condvar& Condvar::operator=(Condvar&&) noexcept = default;

void Condvar::notify_one() noexcept {
  _inn->notify_one();
}

void Condvar::notify_all() noexcept {
  _inn->notify_all();
}

void Condvar::wait(Mutex::Guard& guard) noexcept {
  auto& lock = guard.inner();
  _inn->wait(lock);
}

auto Condvar::wait_timeout(Mutex::Guard& guard, time::Duration dur) noexcept -> bool {
  auto& lock = guard.inner();
  return _inn->wait_timeout(lock, dur);
}

}  // namespace sfc::sync
