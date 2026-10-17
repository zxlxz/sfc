#include "sfc/sys/thread.h"

#include "sfc/thread/join_handle.h"
#include "sfc/ffi/os_str.h"

namespace sfc::thread {

JoinHandle::JoinHandle(SysThread thrd) noexcept : _thrd{thrd} {}

JoinHandle::~JoinHandle() noexcept {
  _thrd.detach();
}

JoinHandle::JoinHandle(JoinHandle&& other) noexcept : _thrd{mem::take(other._thrd)} {}

JoinHandle& JoinHandle::operator=(JoinHandle&& other) noexcept {
  if (this != &other) {
    mem::swap(_thrd, other._thrd);
  }
  return *this;
}

auto JoinHandle::is_finished() const -> bool {
  return _thrd._raw == ThreadHandle{};
}

void JoinHandle::join() {
  _thrd.join();
}

auto Builder::spawn(Box<void()> fun) -> JoinHandle {
  auto thrd = sys::Thread::spawn(stack_size, mem::move(fun));
  return JoinHandle{SysThread{thrd._handle}};
}

}  // namespace sfc::thread
