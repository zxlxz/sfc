#include "sfc/thread.h"

#include "sfc/sys/thread.h"
#include "sfc/ffi/c_str.h"

namespace sfc::thread {

namespace sys_imp = sys::thread;
using ffi::CString;

using sys_imp::thrd_t;

struct ThreadData {
  Box<void()> _func;
  CString _name;

 public:
  void run() noexcept {
    sys_imp::thrd_setname(_name.as_ptr());

    try {
      _func();
    } catch (...) {}
  }
};

auto Builder::spawn(Box<void()> fun) -> JoinHandle {
  const auto call_back = [](void* ptr) -> sys_imp::ret_t {
    auto dat = static_cast<ThreadData*>(ptr);
    auto obj = Box<ThreadData>::from_raw(dat);
    obj->run();
    return {};
  };

  auto data = Box<ThreadData>::xnew(mem::move(fun), CString::xnew(name));
  auto thrd = sys_imp::thrd_create(stack_size, call_back, &*data);
  if (thrd) {
    mem::move(data).into_raw();
  }

  return JoinHandle{Thread{thrd}};
}

JoinHandle::JoinHandle(Thread thrd) noexcept : _thrd{thrd} {}

JoinHandle::~JoinHandle() noexcept {
  if (!_thrd._raw) {
    return;
  }
  sys_imp::thrd_join(static_cast<thrd_t>(_thrd._raw));
}

auto current() -> Thread {
  const auto thrd = sys_imp::thrd_current();
  return Thread{thrd};
}

void yield_now() {
  sys_imp::thrd_yield();
}

void sleep(time::Duration dur) {
  const auto millis = static_cast<u32>(dur.as_millis());
  sys_imp::thrd_sleep_ms(millis);
}

void sleep_ms(u32 ms) {
  sys_imp::thrd_sleep_ms(ms);
}

}  // namespace sfc::thread
