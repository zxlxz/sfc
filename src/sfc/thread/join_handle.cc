#include "sfc/sys/thread.h"

#include "sfc/thread/join_handle.h"
#include "sfc/ffi/os_str.h"

namespace sfc::thread {

struct JoinHandle::Inn {
  sys::Thread _native;

 public:
  explicit Inn(usize stack_size, Box<void()> f) {
    _native = sys::Thread::new_(stack_size, mem::move(f));
  }

  ~Inn() {}

 public:
  void detach() {
    _native.detach();
  }

  void join() {
    _native.join();
  }
};

JoinHandle::JoinHandle(Inn inn) noexcept : _inn{new Inn{mem::move(inn)}} {}

JoinHandle::~JoinHandle() noexcept {
  if (_inn == nullptr) {
    return;
  }

  _inn->detach();
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

  _inn->join();
  delete _inn;
  _inn = nullptr;
}

auto Builder::spawn_unchecked(Box<void()> fun) -> JoinHandle {
  const auto inn = JoinHandle::Inn{stack_size, mem::move(fun)};
  return JoinHandle{inn};
}

Scope::Scope() {}

Scope::~Scope() {
  for (auto& handle : _handles) {
    handle.join();
  }
}

}  // namespace sfc::thread
