#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/sync.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/sync.inl"
#endif

#include "sfc/sync/condvar.h"

namespace sfc::sync {

Condvar::Condvar() noexcept {}

Condvar::~Condvar() noexcept {}

Condvar::Condvar(Condvar&&) noexcept = default;

Condvar& Condvar::operator=(Condvar&&) noexcept = default;

void Condvar::notify_one() noexcept {
  _inn.notify_one();
}

void Condvar::notify_all() noexcept {
  _inn.notify_all();
}

auto Condvar::wait(Mutex::Guard& guard) noexcept -> bool {
  auto& lock = guard.inner();
  _inn.wait(lock);
  return true;
}

auto Condvar::wait_timeout(Mutex::Guard& guard, time::Duration dur) noexcept -> bool {
  auto& lock = guard.inner();
  return _inn.wait_timeout(lock, dur);
}

}  // namespace sfc::sync
