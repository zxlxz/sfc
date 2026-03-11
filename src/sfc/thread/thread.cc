#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/thread.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/thread.inl"
#endif

#define _SFC_SYS_THREAD_
#include "sfc/thread.h"
#include "sfc/ffi/os_str.h"

namespace sfc::thread {

struct ThreadData {
  Box<void()> _func;
  ffi::OsString _name;

 public:
  void run() noexcept {
    const auto name_ptr = _name.ptr();
    if (name_ptr != nullptr) {
      sys::Thread::set_name(name_ptr);
    }

    try {
      _func();
    } catch (...) {
      __builtin_abort();
    }
  }

  static auto callback(void* ptr) -> sys::Thread::ret_t {
    auto dat = static_cast<ThreadData*>(ptr);
    auto obj = Box<ThreadData>::from_raw(dat);
    obj->run();
    return {};
  }
};

auto Thread::id() const -> u32 {
  return _inn.tid();
}

JoinHandle::JoinHandle() noexcept = default;

JoinHandle::~JoinHandle() noexcept {
  if (_thread.is_valid()) {
    _thread.join();
  }
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
  auto thrd = sys::Thread::spawn(stack_size, &ThreadData::callback, data.ptr());
  if (thrd.is_valid()) {  // forget data if spawn success
    mem::move(data).into_raw();
  }

  auto res = JoinHandle{};
  res._thread = sys::Thread{thrd};
  return res;
}

auto current() -> Thread {
  const auto thrd = sys::Thread::current();
  return Thread{thrd};
}

void yield_now() {
  sys::Thread::yield();
}

void sleep(time::Duration dur) {
  const auto millis = static_cast<u32>(dur.as_millis());
  sys::sleep_ms(millis);
}

void sleep_ms(u32 ms) {
  sys::sleep_ms(ms);
}

}  // namespace sfc::thread
