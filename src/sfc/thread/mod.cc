#include "mod.h"

#include "sfc/ffi.h"
#include "sfc/sys/thread.inl"

namespace sfc::thread {

namespace sys_imp = sys::thread;

auto Thread::current() -> Thread {
  const auto imp = sys_imp::Thread::current();
  return Thread{imp.id()};
}

auto Thread::id() const -> u64 {
  return _id;
}

auto Thread::name() const -> String {
  const auto imp = sys_imp::Thread{_id};
  return imp.name();
}

auto Builder::spawn(Box<void()> f) -> JoinHandle {
  auto fun = [f = mem::move(f), name = ffi::CString::from(_name)]() mutable {
    auto thr = sys_imp::Thread::current();
    thr.set_name(name);
    f();
  };

  auto imp = sys_imp::Thread::xnew(_stack_size, Box<void()>{mem::move(fun)})
                 .expect("sys::Thread::Builder: spawn thread failed.");

    return JoinHandle{Thread{imp.id()}};
}

JoinHandle::JoinHandle() : _thr{} {}

JoinHandle::JoinHandle(Thread thr) : _thr{thr} {}

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

auto JoinHandle::thread() const -> Thread {
    return _thr;
}

void JoinHandle::join() {
    if (_thr._id == Thread::INVALID_ID) {
      return;
    }

    auto imp = sys_imp::Thread{_thr._id};
    imp.join();
}

void sleep(time::Duration dur) {
    sys_imp::sleep(dur);
}

}  // namespace sfc::thread
