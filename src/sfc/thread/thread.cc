#include "thread.h"

#include "sfc/sys/thread.h"
#include "sfc/ffi/cstring.h"

namespace sfc::thread {

namespace sys_imp = sys::thread;

#ifdef _WIN32
using thread_ret_t = unsigned long;
#else
using thread_ret_t = void*;
#endif

static auto start_routine(void* raw) -> thread_ret_t {
  const auto ptr = static_cast<Box<void()>*>(raw);
  auto       fun = Box<Box<void()>>::from_raw(ptr);

  try {
    (**fun)();
  } catch (...) {}

  return 0;
};

auto Thread::current() -> Thread {
  const auto thrd_imp = sys_imp::Thread::current();
  return Thread{thrd_imp._raw};
}

auto Thread::id() const -> i64 {
  const auto thrd_imp = sys_imp::Thread{_raw};
  return thrd_imp.id();
}

auto Thread::name() const -> String {
  auto res = String{};
  res.reserve(64);

  const auto thrd_imp = sys_imp::Thread{_raw};
  const auto name_len = thrd_imp.get_name(res.as_mut_ptr(), res.capacity());
  res.set_len(name_len);

  return res;
}

auto Builder::spawn(Box<void()> fun) const -> JoinHandle {
  auto fun_imp = Box<void()>::xnew([fun = mem::move(fun), name = ffi::CString::from(name.as_str())]() mutable {
    auto thrd_imp = sys_imp::Thread::current();
    thrd_imp.set_name(name.c_str());
    (*fun)();
  });

  auto fun_box = boxed::box(mem::move(fun_imp));
  auto sys_thr = sys_imp::Thread::start(stack_size, start_routine, fun_box.ptr());
  if (sys_thr) {
    mem::move(fun_box).into_raw();
  }

  auto res = JoinHandle{};
  res._thrd = Thread{sys_thr._raw};
  return res;
}

JoinHandle::JoinHandle() noexcept = default;

JoinHandle::~JoinHandle() {
  auto thr = sys_imp::Thread{_thrd._raw};
  thr.detach();
}

JoinHandle::JoinHandle(JoinHandle&& other) noexcept : _thrd{other._thrd} {
  other._thrd._raw = thrd_t(0);
}

JoinHandle& JoinHandle::operator=(JoinHandle&& other) noexcept {
  auto tmp = mem::move(other);
  mem::swap(_thrd, tmp._thrd);
  return *this;
}

void JoinHandle::join() {
  const auto tid = mem::take(_thrd._raw);
  sys_imp::Thread{tid}.join();
}

void sleep(const time::Duration& dur) {
  sys_imp::sleep_ms(dur.as_millis());
}

}  // namespace sfc::thread
