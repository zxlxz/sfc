#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/thread.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/thread.inl"
#endif

#include "sfc/thread/join_handle.h"
#include "sfc/ffi/os_str.h"

namespace sfc::thread {

struct ThreadData {
  Box<void()> _func;
  ffi::OsString _name;

 public:
  void run() noexcept {
    const auto name_ptr = _name.as_ptr();
    if (name_ptr != nullptr) {
      sys::Thread::set_name(name_ptr);
    }
    _func();
  }

  static auto run(void* ptr) noexcept -> bool {
    auto dat = ptr::cast<ThreadData>(ptr);
    auto obj = Box<ThreadData>::from_raw(dat);

    try {
      obj->run();
    } catch (...) {
      return false;
    }

    return true;
  }
};

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
  auto data = Box<ThreadData>::new_(mem::move(fun), ffi::OsString::from(name));
  auto thrd = sys::Thread::spawn(stack_size, data.ptr());
  // if thread creation succeeded:
  //    the thread will take ownership of data
  //    so we should forget it here to avoid double free
  if (thrd.is_valid()) {
    mem::forget(data);
  }

  return JoinHandle{sys::Thread{thrd}};
}

}  // namespace sfc::thread
