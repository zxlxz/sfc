#pragma once

#include "sfc/sys/windows/mod.inl"
#define _SFC_SYS_THREAD_

namespace sfc::sys::windows {

struct Thread {
  HANDLE _raw = nullptr;

 public:
  static auto current() -> DWORD {
    const auto tid = ::GetCurrentThreadId();
    return tid;
  }

  template <class Fn>
  static unsigned __stdcall callback(void* p) {
    return Fn::run(p) ? 0 : 1;
  }

  template <class Fn>
  static auto spawn(size_t stack_size, Fn* func) -> Thread {
    const auto thrd = _beginthreadex(nullptr, stack_size, callback<Fn>, func, 0, nullptr);
    if (thrd == 0) {
      return {};
    }
    return Thread{reinterpret_cast<HANDLE>(thrd)};
  }

  static void yield_now() {
    (void)::SwitchToThread();
  }

  static auto set_name(const wchar_t* name) -> bool {
    if (name == nullptr) {
      return true;
    }

    const auto thrd = ::GetCurrentThread();
    const auto hres = ::SetThreadDescription(thrd, name);
    return SUCCEEDED(hres);
  }

  auto is_valid() const -> bool {
    return _raw != nullptr && _raw != INVALID_HANDLE_VALUE;
  }

  void join() {
    sfc::expect(this->is_valid(), "Thread::join: invalid thread");

    const auto ret = ::WaitForSingleObject(_raw, INFINITE);
    ::CloseHandle(_raw);
    sfc::expect(ret == WAIT_OBJECT_0,
                "Thread::join: WaitForSingleObject failed, err={}",
                ::GetLastError());
  }

  void detach() {
    sfc::expect(this->is_valid(), "Thread::detach: invalid thread");
    (void)::CloseHandle(_raw);
  }
};

inline void sleep(time::Duration dur) {
  const auto millis = static_cast<DWORD>(dur.as_millis());

  if (millis == 0) {
    ::SwitchToThread();
    return;
  }

  ::Sleep(millis);
}

}  // namespace sfc::sys::windows
