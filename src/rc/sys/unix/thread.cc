#include "rc/sys/unix.inl"

namespace rc::sys::unix::thread {

using boxed::FnBox;

static auto thread_start(void* p) -> void* {
  auto raw = ptr::cast<FnBox<void()>::Fn>(p);
  auto box = FnBox<void()>::from_raw(raw);
  box();
  return nullptr;
}

auto Thread::spawn(FnBox<void()> f) -> Thread {
  auto thr = ::pthread_t(nullptr);
  auto arg = rc::move(f).into_raw();
  const auto eid = ::pthread_create(&thr, nullptr, &thread_start, arg);

  if (eid != 0) {
    throw io::Error::from_raw_os_error(eid);
  }

  return Thread{thr};
}

auto Thread::join() -> void {
  void* res = nullptr;
  const auto eid = ::pthread_join(_raw, &res);
  if (eid != 0) {
    throw io::Error::from_raw_os_error(eid);
  }
}

auto Thread::yield_now() -> void {
  const auto eid = ::sched_yield();
  if (eid != 0) {
    throw io::Error::from_raw_os_error(eid);
  }
}

auto Thread::sleep(time::Duration dur) -> void {
  auto ts = ::timespec{
      time_t(dur._secs),
      long(dur._nanos),
  };

  while (ts.tv_sec > 0 || ts.tv_nsec > 0) {
    auto ret = ::nanosleep(&ts, &ts);
    if (ret == -1) return;
  }
}

}  // namespace rc::sys::unix::thread
