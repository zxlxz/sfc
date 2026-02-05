#include "sfc/thread.h"

#include "sfc/sys/thread.h"

namespace sfc::thread {

namespace sys_imp = sys::thread;

struct ThreadData {
  Box<void()> _func;
  sys::OsStr _name;

 public:
  void run() noexcept {
    const auto name_ptr = _name.ptr();
    if (name_ptr != nullptr) {
      sys_imp::thrd_setname(name_ptr);
    }

    try {
      _func();
    } catch (...) {
      __builtin_abort();
    }
  }
};

static auto thread_callback(void* ptr) -> sys_imp::ret_t {
  auto dat = static_cast<ThreadData*>(ptr);
  auto obj = Box<ThreadData>::from_raw(dat);
  obj->run();
  return {};
}

void Thread::join() {
  if (!_raw) {
    return;
  }
  sys_imp::thrd_join(static_cast<sys_imp::thrd_t>(_raw));
}

auto Builder::spawn(Box<void()> fun) -> JoinHandle {
  auto data = Box<ThreadData>::xnew(mem::move(fun), sys::OsStr::xnew(name));
  auto thrd = sys_imp::thrd_create(stack_size, thread_callback, data.ptr());
  if (thrd) {
    // forget data
    mem::move(data).into_raw();
  }

  auto res = JoinHandle{};
  res._thrd = Thread{thrd};
  return res;
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
