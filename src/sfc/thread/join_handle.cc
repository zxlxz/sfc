#include "sfc/thread/join_handle.h"
#include "sfc/ffi/os_str.h"
#include "sfc/sys.h"

namespace sfc::thread {

JoinHandle::JoinHandle(sys::Thread thread) noexcept : _thread{thread} {}

JoinHandle::~JoinHandle() noexcept {}

JoinHandle::JoinHandle(JoinHandle&& other) noexcept : _thread{other._thread} {
  other._thread = {};
}

JoinHandle& JoinHandle::operator=(JoinHandle&& other) noexcept {
  if (this == &other) {
    return *this;
  }
  mem::swap(_thread, other._thread);
  return *this;
}

auto JoinHandle::is_finished() const -> bool {
  return !_thread.is_valid();
}

void JoinHandle::join() {
  if (!_thread.is_valid()) {
    return;
  }

  _thread.join();
  _thread = {};
}

JoinGuard::JoinGuard(JoinHandle handle) noexcept : _handle{mem::move(handle)} {}

JoinGuard::~JoinGuard() noexcept {
  _handle.join();
}

JoinGuard::JoinGuard(JoinGuard&&) noexcept = default;

JoinGuard& JoinGuard::operator=(JoinGuard&&) noexcept = default;

auto Builder::spawn(Box<void()> fun) -> JoinHandle {
  auto thrd = sys::Thread::spawn(stack_size, mem::move(fun));
  return JoinHandle{sys::Thread{thrd}};
}

}  // namespace sfc::thread
