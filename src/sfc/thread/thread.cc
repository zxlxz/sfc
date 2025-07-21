#include "sfc/thread/thread.h"

#include "sfc/ffi/cstring.h"
#include "sfc/sys/thread.h"

namespace sfc::thread {

namespace sys_imp = sys::thread;

struct ThreadData {
  Box<void()> _fbox;
  ffi::CString _name;

 public:
  void run() {
    if (_name) {
      sys_imp::set_name(_name.c_str());
    }

    try {
      _fbox();
    } catch (...) {}
  }
};

static auto start_routine(void* data) -> sys_imp::thrd_ret_t {
  auto obj = Box<ThreadData>::from_raw(static_cast<ThreadData*>(data));
  obj->run();

  return {};
};

auto Thread::current() -> Thread {
  const auto thrd = sys_imp::current();
  return Thread{thrd};
}

auto Thread::name() const -> String {
  char buf[256];

  const auto name = sys_imp::get_name(_raw, buf);
  return String::from_cstr(name);
}

void Thread::join() {
  if (_raw == 0) {
    return;
  }

  sys_imp::join(_raw);
  _raw = 0;
}

auto Builder::spawn(Box<void()> fun) const -> JoinHandle {
  auto data = Box<ThreadData>::xnew(mem::move(fun), ffi::CString::from(name));
  auto thrd = sys_imp::start(stack_size, start_routine, &*data);
  if (thrd) {
    mem::move(data).into_raw();
  }

  return JoinHandle{Thread{thrd}};
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
  const auto nanos = dur.as_nanos();
  sys_imp::sleep_ns(nanos);
}

}  // namespace sfc::thread
