#include "rc/sys/windows.inl"

namespace rc::sys::windows::thread {

static DWORD __stdcall thread_start(void* arg) {
  const auto raw = ptr::cast<boxed::FnBox<void()>::Inner>(arg);
  auto box = boxed::FnBox<void()>::from_raw(raw);
  (*box)();
  return 0;
}

auto Thread::spawn(usize stack_size, FnBox call_back) -> Thread {
  auto tid = ::DWORD(0);
  auto raw = rc::move(call_back).into_raw();
  auto thr = ::CreateThread(nullptr, stack_size, &thread_start, raw, 0, &tid);
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
