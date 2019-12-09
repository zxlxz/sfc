#include "rc/sys/windows.inl"

namespace rc::sys::windows::thread {

using boxed::FnBox;

static auto thread_start(void* arg) -> DWORD {
  auto raw = ptr::cast<FnBox<void()>::Fn>(arg);
  auto box = FnBox<void()>::from_raw(raw);
  box();
  return 0;
}

auto Thread::spawn(FnBox<void()> f) -> Thread {
  const auto raw = rc::move(f).into_raw();
  const auto thr = ::CreateThread(nullptr, 0, &thread_start, raw, 0, nullptr);

  if (thr == nullptr) {
    throw io::Error::last_os_error();
  }

  return Thread{thr};
};

auto Thread::join() -> void {
  const auto rc = ::WaitForSingleObject(_handle, INFINITE);

  if (rc == WAIT_FAILED) {
    throw io::Error::last_os_error();
  }
}

auto Thread::yield_now() -> void {
  const auto res = ::SwitchToThread();
  (void)res;
}

auto Thread::sleep(time::Duration dur) -> void {
  const auto ms = dur.as_millis_f64();
  ::Sleep(u32(ms));
}

}  // namespace rc::sys::windows::thread
