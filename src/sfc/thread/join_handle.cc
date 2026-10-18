#include "sfc/sys/thread.h"

#include "sfc/thread/join_handle.h"
#include "sfc/ffi/os_str.h"

namespace sfc::thread {

struct JoinHandle::Inn {
  sys::Thread _native;
};

JoinHandle::JoinHandle(Inn inn) noexcept : _inn{new Inn{inn}} {}

JoinHandle::~JoinHandle() noexcept {
  if (_inn == nullptr) {
    return;
  }

  _inn->_native.detach();
  delete _inn;
}

JoinHandle::JoinHandle(JoinHandle&& other) noexcept : _inn{mem::take(other._inn)} {}

JoinHandle& JoinHandle::operator=(JoinHandle&& other) noexcept {
  if (this != &other) {
    mem::swap(_inn, other._inn);
  }
  return *this;
}

void JoinHandle::join() {
  if (_inn == nullptr) {
    return;
  }
  _inn->_native.join();

  delete _inn;
  _inn = nullptr;
}

auto Builder::spawn_imp(Box<void()> fun) -> JoinHandle {
  auto thrd = sys::Thread::spawn(stack_size, mem::move(fun));
  auto inn = JoinHandle::Inn{._native = mem::move(thrd)};
  return JoinHandle{mem::move(inn)};
}

Scope::Scope() {}

Scope::~Scope() {
  for (auto& handle : _handles) {
    handle.join();
  }
}

}  // namespace sfc::thread
