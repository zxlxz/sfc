#include "sfc/core/mem.h"
#include "sfc/sys/thread.h"

#include "sfc/thread/join_handle.h"
#include "sfc/ffi/os_str.h"

namespace sfc::thread {

// private func: cast from Inn to sys::Thread*
auto* JoinHandle::Inn::operator->() {
  static_assert(sizeof(Inn) == sizeof(sys::Thread));
  static_assert(alignof(Inn) == alignof(sys::Thread));
  return (sys::Thread*)this;
}

JoinHandle::JoinHandle(Inn inn) noexcept : _inn{inn} {}

JoinHandle::~JoinHandle() noexcept {
  if (_inn._native == RawThread{}) {
    return;
  }

  _inn->detach();
}

JoinHandle::JoinHandle(JoinHandle&& other) noexcept : _inn{other._inn} {
  other._inn._native = RawThread{};
}

JoinHandle& JoinHandle::operator=(JoinHandle&& other) noexcept {
  if (this != &other) {
    mem::swap(_inn, other._inn);
  }
  return *this;
}

void JoinHandle::join() {
  if (_inn._native == RawThread{}) {
    return;
  }

  _inn->join();
  _inn._native = {};
}

auto Builder::spawn_unchecked(Box<void()> fun) -> JoinHandle {
  auto sys_thread = sys::Thread::new_(stack_size, mem::move(fun));
  return JoinHandle{{sys_thread._handle}};
}

Scope::Scope() {}

Scope::~Scope() {
  for (auto& handle : _handles) {
    handle.join();
  }
}

}  // namespace sfc::thread
