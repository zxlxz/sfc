#include "sfc/thread/thread.h"

#include "sfc/ffi/cstring.h"
#include "sfc/sys/thread.h"

namespace sfc::thread {

namespace sys_imp = sys::thread;
using sys_imp::thread_t;

struct ThreadData {
  Box<void()> _fbox;
  ffi::CString _name;

 public:
  void run() {
    if (_name) {
      sys_imp::Thread::set_name(_name.c_str());
    }

    try {
      _fbox();
    } catch (...) {}
  }
};

static auto start_routine(void* data) -> sys_imp::thread_ret_t {
  auto obj = Box<ThreadData>::from_raw(static_cast<ThreadData*>(data));
  obj->run();

  return {};
};

auto Thread::current() -> Thread {
  const auto imp = sys_imp::Thread::current();
  return Thread{imp._raw};
}

auto Thread::name() const -> String {
  char buf[256];

  const auto imp = sys_imp::Thread{static_cast<thread_t>(_raw)};
  const auto name = imp.get_name(buf);
  return String::from_cstr(name);
}

void Thread::join() {
  if (_raw == 0) {
    return;
  }

  auto imp = sys_imp::Thread{static_cast<thread_t>(_raw)};
  imp.join();
  _raw = 0;
}

auto Builder::spawn(Box<void()> fun) const -> JoinHandle {
  auto thr_box = Box<ThreadData>::xnew(mem::move(fun), ffi::CString::from(name));
  auto sys_thr = sys_imp::Thread::start(stack_size, start_routine, &*thr_box);
  if (sys_thr) {
    mem::move(thr_box).into_raw();
  }

  auto thrd = Thread{sys_thr._raw};
  return JoinHandle{thrd};
}

JoinHandle::JoinHandle(Thread thrd) noexcept : _thrd{thrd} {}

JoinHandle::~JoinHandle() {
  if (!_thrd._raw) {
    return;
  }
  _thrd.join();
}

JoinHandle::JoinHandle(JoinHandle&& other) noexcept : _thrd{mem::take(other._thrd)} {}

JoinHandle& JoinHandle::operator=(JoinHandle&& other) noexcept {
  auto tmp = static_cast<JoinHandle&&>(*this);
  _thrd = mem::take(other._thrd);
  return *this;
}

void sleep(const time::Duration& dur) {
  sys_imp::sleep_ms(dur.as_millis());
}

}  // namespace sfc::thread
