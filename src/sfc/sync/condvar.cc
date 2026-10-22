#include "sfc/sys/sync.h"
#include "sfc/sync/condvar.h"

namespace sfc::sync {

class Mutex::Inn : public sys::Mutex {};

class Condvar::Inn : public sys::Condvar {};

Condvar::Condvar() noexcept : _inn{new Inn()} {}

Condvar::~Condvar() noexcept {
  if (_inn == nullptr) {
    return;
  }

  delete _inn;
}

Condvar::Condvar(Condvar&& other) noexcept : _inn{mem::take(other._inn)} {}

Condvar& Condvar::operator=(Condvar&& other) noexcept {
  if (this != &other) {
    mem::swap(_inn, other._inn);
  }
  return *this;
}

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
