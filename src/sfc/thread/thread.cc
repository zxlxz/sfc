#include "sfc/thread/thread.h"

#include "sfc/ffi/cstring.h"
#include "sfc/sys/thread.h"

namespace sfc::thread {

namespace sys_imp = sys::thread;

struct ThreadData {
  Box<void()>  _fbox;
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
  return Thread{imp.raw()};
}

auto Thread::name() const -> String {
  auto res = String{};
  res.reserve(64);

  const auto imp      = sys_imp::Thread::from_raw(_raw);
  const auto name_len = imp.get_name(res.as_mut_ptr(), res.capacity());
  res.set_len(name_len);

  return res;
}

auto Builder::spawn(Box<void()> fun) const -> JoinHandle {
  auto thr_box = Box<ThreadData>::xnew(mem::move(fun), ffi::CString::from(name));
  auto sys_thr = sys_imp::Thread::start(stack_size, start_routine, &*thr_box);
  if (sys_thr) {
    mem::move(thr_box).into_raw();
  }

  auto res  = JoinHandle{};
  res._thrd = Thread{sys_thr.raw()};
  return res;
}

JoinHandle::JoinHandle() noexcept = default;

JoinHandle::~JoinHandle() {
  auto thr = sys_imp::Thread::from_raw(_thrd._raw);
  if (!thr) {
    return;
  }
  thr.detach();
}

JoinHandle::JoinHandle(JoinHandle&& other) noexcept : _thrd{other._thrd} {
  other._thrd = Thread{};
}

JoinHandle& JoinHandle::operator=(JoinHandle&& other) noexcept {
  auto tmp = mem::move(other);
  mem::swap(_thrd, tmp._thrd);
  return *this;
}

void JoinHandle::join() {
  auto imp = sys_imp::Thread::from_raw(_thrd._raw);
  _thrd    = Thread{};
  imp.join();
}

void sleep(const time::Duration& dur) {
  sys_imp::sleep_ms(dur.as_millis());
}

}  // namespace sfc::thread
