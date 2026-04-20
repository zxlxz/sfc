#pragma once

#include "sfc/sys/windows/mod.inl"
#define _SFC_SYS_THREAD_

namespace sfc::sys::windows {

struct Thread {
  HANDLE _thrd = nullptr;

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

  static auto yield_now() -> bool {
    const auto ret = ::SwitchToThread();
    return bool(ret);
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
    return _thrd != nullptr && _thrd != INVALID_HANDLE_VALUE;
  }

  auto join() -> bool {
    if (_thrd == nullptr) {
      return true;
    }

    const auto ret = ::WaitForSingleObject(_thrd, INFINITE);
    ::CloseHandle(_thrd);
    _thrd = nullptr;
    return ret == WAIT_OBJECT_0;
  }

  auto detach() -> bool {
    if (_thrd == nullptr) {
      return true;
    }
    const auto ret = ::CloseHandle(_thrd);
    _thrd = nullptr;
    return bool(ret);
  }
};

inline auto sleep_ms(unsigned millis) -> bool {
  if (millis == 0) {
    return bool(::SwitchToThread());
  }

  ::Sleep(millis);
  return true;
}

}  // namespace sfc::sys::windows
