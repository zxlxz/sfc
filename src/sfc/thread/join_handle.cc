#include "sfc/sys/thread.h"

#include "sfc/thread/join_handle.h"
#include "sfc/ffi/os_str.h"

namespace sfc::thread {

JoinHandle::JoinHandle(SysThread thrd) noexcept : _thrd{thrd} {}

JoinHandle::~JoinHandle() noexcept {
  this->join();
}

JoinHandle::JoinHandle(JoinHandle&& other) noexcept : _thrd{other._thrd} {
  other._thrd = {};
}

JoinHandle& JoinHandle::operator=(JoinHandle&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  mem::swap(_thrd, other._thrd);
  return *this;
}

auto JoinHandle::is_finished() const -> bool {
  return _thrd._raw == ThreadHandle{};
}

void JoinHandle::join() {
  if (this->is_finished()) {
    return;
  }

  _thrd.join();
  _thrd = {};
}

JoinGuard::JoinGuard(JoinHandle handle) noexcept : _handle{mem::move(handle)} {}

JoinGuard::~JoinGuard() noexcept {
  _handle.join();
}

JoinGuard::JoinGuard(JoinGuard&&) noexcept = default;

JoinGuard& JoinGuard::operator=(JoinGuard&&) noexcept = default;

auto Builder::spawn(Box<void()> fun) -> JoinHandle {
  auto thrd = sys::Thread::spawn(stack_size, mem::move(fun));
  return JoinHandle{SysThread{thrd._handle}};
}

}  // namespace sfc::thread
