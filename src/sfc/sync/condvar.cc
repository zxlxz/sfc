#include "sfc/sync/condvar.h"
#include "sfc/sys/sync.h"

namespace sfc::sync {

struct Mutex::Inn : sys::Mutex {};

struct Condvar::Inn : sys::Condvar {};

Condvar::Condvar() noexcept : _inn{new Inn{}} {
  _inn->init();
}

Condvar::~Condvar() noexcept {
  if (_inn == nullptr) {
    return;
  }

  _inn->destroy();
  delete _inn;
  _inn = nullptr;
}

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
