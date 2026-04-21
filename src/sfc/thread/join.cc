#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/thread.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/thread.inl"
#endif

#include "sfc/thread/join.h"
#include "sfc/ffi/os_str.h"

namespace sfc::thread {

struct ThreadData {
  Box<void()> _func;
  ffi::OsString _name;

 public:
  void run() noexcept {
    const auto os_name = _name.ptr();
    if (os_name != nullptr) {
      sys::Thread::set_name(os_name);
    }
    _func();
  }

  static auto run(void* ptr) noexcept -> bool {
    auto dat = static_cast<ThreadData*>(ptr);
    auto obj = Box<ThreadData>::from_raw(dat);

    try {
      obj->run();
    } catch (...) {
      return false;
    }

    return true;
  }
};

JoinHandle::JoinHandle() noexcept {}

JoinHandle::~JoinHandle() noexcept {
  if (!_thread.is_valid()) return;
  _thread.join();
}

JoinHandle::JoinHandle(JoinHandle&& other) noexcept : _thread{other._thread} {
  other._thread = {};
}

JoinHandle& JoinHandle::operator=(JoinHandle&& other) noexcept {
  if (this != &other) {
    mem::swap(_thread, other._thread);
  }
  return *this;
}

auto Builder::spawn(Box<void()> fun) -> JoinHandle {
  auto data = Box<ThreadData>::xnew(mem::move(fun), ffi::OsString::from(name));
  auto thrd = sys::Thread::spawn(stack_size, data.ptr());
  // if thread creation succeeded:
  //    the thread will take ownership of data
  //    so we should forget it here to avoid double free
  if (thrd.is_valid()) {
    mem::forget(data);
  }

  auto res = JoinHandle{};
  res._thread = sys::Thread{thrd};
  return res;
}

}  // namespace sfc::thread
