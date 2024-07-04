#include "sfc/thread.h"

#include "sfc/sys/thread.h"

namespace sfc::thread {

namespace sys_imp = sys::thread;

using sys_imp::thrd_t;

struct ThreadData {
  Box<void()> _func;
  CString _name;

 public:
  void run() noexcept {
    sys_imp::set_name(_name);

    try {
      _func();
    } catch (...) {}
  }
};

static auto start_routine(void* data) -> sys_imp::ret_t {
  auto obj = Box<ThreadData>::from_raw(static_cast<ThreadData*>(data));
  obj->run();
  return {};
};

auto Builder::spawn(Box<void()> fun) -> JoinHandle {
  auto data = Box<ThreadData>::xnew(mem::move(fun), CString::from(name));
  auto thrd = sys_imp::start(stack_size, start_routine, &*data);
  if (thrd) {
    mem::move(data).into_raw();
  }

  return JoinHandle{Thread{thrd}};
}

JoinHandle::JoinHandle(Thread thrd) noexcept : _thrd{thrd} {}

JoinHandle::~JoinHandle() noexcept{
  if (!_thrd._raw) {
    return;
  }
  sys_imp::join(static_cast<thrd_t>(_thrd._raw));
}

auto current() -> Thread {
  const auto thrd = sys_imp::current();
  return Thread{thrd};
}

void yield_now() {
  sys_imp::yield();
}

void sleep(time::Duration dur) {
  const auto millis = static_cast<u32>(dur.as_millis());
  sys_imp::sleep_ms(millis);
}

void sleep_ms(u32 ms) {
  sys_imp::sleep_ms(ms);
}

}  // namespace sfc::thread
