#include "sfc/thread.h"

#include "sfc/sys/thread.h"

namespace sfc::thread {

namespace sys_imp = sys::thread;

using sys_imp::thrd_t;

struct ThreadData {
  Box<void()> _fbox;
  CString _name;

 public:
  void run() {
    sys_imp::set_name(_name);

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

  const auto name = Str::from(sys_imp::get_name(static_cast<thrd_t>(_raw), buf));
  return String::from(name);
}

void Thread::join() {
  if (_raw == 0) {
    return;
  }

  sys_imp::join(static_cast<thrd_t>(_raw));
  _raw = 0;
}

auto Builder::spawn(Box<void()> fun) const -> JoinHandle {
  auto data = Box<ThreadData>::xnew(mem::move(fun), CString::from(name));
  auto thrd = sys_imp::start(stack_size, start_routine, &*data);
  if (thrd) {
    mem::move(data).into_raw();
  }

  return JoinHandle{Thread{thrd}};
}

JoinHandle::JoinHandle(Thread thrd) noexcept : _thrd{thrd} {}

JoinHandle::~JoinHandle() {
  if (_thrd._raw) {
    _thrd.join();
  }
}

JoinHandle::JoinHandle(JoinHandle&& other) noexcept : _thrd{mem::take(other._thrd)} {}

JoinHandle& JoinHandle::operator=(JoinHandle&& other) noexcept {
  if (_thrd._raw) {
    _thrd.join();
  }
  _thrd = mem::take(other._thrd);
  return *this;
}

void sleep(const time::Duration& dur) {
  const auto nanos = dur.as_nanos();
  sys_imp::sleep_ns(nanos);
}

}  // namespace sfc::thread
