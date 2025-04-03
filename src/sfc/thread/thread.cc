#include "sfc/sys/thread.inl"

#include "thread.h"

namespace sfc::thread {

namespace sys_imp = sys::thread;

auto Thread::current() -> Thread {
  const auto imp = sys_imp::Thread::current();
  return Thread{imp.raw()};
}

auto Thread::name() const -> String {
  const auto imp = sys_imp::Thread{_raw};
  return imp.name();
}

auto Builder::spawn(Box<void()> f) -> JoinHandle {
  auto fun = [f = mem::move(f), name = mem::move(name)]() mutable {
    auto thr = sys_imp::Thread::current();
    thr.set_name(name.c_str());
    try {
      f();
    } catch (...) {
      return;
    }
  };

  auto imp = sys_imp::Thread::xnew(stack_size, Box<void()>::xnew(mem::move(fun)))
                 .expect("sys::Thread::Builder: spawn thread failed.");

  auto res = JoinHandle{};
  res._thr = Thread{imp.raw()};
  return res;
}

JoinHandle::JoinHandle() noexcept = default;

JoinHandle::~JoinHandle() {
  this->join();
}

JoinHandle::JoinHandle(JoinHandle&& other) noexcept : _thr{other._thr} {
  other._thr = {};
}

auto JoinHandle::operator=(JoinHandle&& other) noexcept -> JoinHandle& {
  auto tmp = mem::move(*this);
  mem::swap(_thr, other._thr);
  return *this;
}

void JoinHandle::join() {
  if (_thr._raw != thrd_t(0)) {
    return;
  }

  auto imp = sys_imp::Thread{_thr._raw};
  imp.join();
}

void sleep(const time::Duration& dur) {
  sys_imp::sleep_ms(dur.as_millis());
}

}  // namespace sfc::thread
