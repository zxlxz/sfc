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
    const auto ss = num::saturating_cast<u32>(stack_size);
    const auto cb = &Thread::callback<Fn>;
    const auto thrd = ::_beginthreadex(nullptr, ss, cb, func, 0, nullptr);
    if (thrd == 0) {
      return {};
    }
    return Thread{__builtin_bit_cast(HANDLE, thrd)};
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
    if (!this->is_valid()) return;

    const auto ret = ::WaitForSingleObject(_raw, INFINITE);
    ::CloseHandle(_raw);
    sfc::assert_fmt(ret == WAIT_OBJECT_0,
                    fmt::Args{"Thread::join: WaitForSingleObject failed, err={}", ::GetLastError()});
  }

  void detach() {
    if (!this->is_valid()) return;
    (void)::CloseHandle(_raw);
  }
};

inline void sleep(time::Duration dur) {
  const auto millis = num::saturating_cast<DWORD>(dur.as_millis());

  if (millis == 0) {
    ::SwitchToThread();
    return;
  }

  ::Sleep(millis);
}

}  // namespace sfc::sys::windows
