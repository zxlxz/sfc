#pragma once

#include "sfc/sys/windows/mod.inl"
#define _SFC_SYS_THREAD_

namespace sfc::sys::windows {

struct Thread {
  using func_t = DWORD (*)(void*);
  HANDLE _handle = nullptr;

 public:
  static auto current() -> DWORD {
    const auto tid = ::GetCurrentThreadId();
    return tid;
  }

  template <class Fn>
  static auto spawn(size_t stack_size, Fn* func) -> Thread {
    auto callback = [](void* p) -> DWORD { return Fn::run(p) ? 0 : 1; };

    auto tid = DWORD{0};
    auto handle = ::CreateThread(nullptr, stack_size, callback, func, 0, &tid);
    if (handle == nullptr) {
      return {};
    }
    return Thread{handle};
  }

  static auto yield_now() -> bool {
    const auto ret = ::SwitchToThread();
    return bool(ret);
  }

  static auto set_name(const wchar_t* name) -> bool {
    if (name == nullptr) return true;

    const auto thrd = ::GetCurrentThread();
    const auto hres = ::SetThreadDescription(thrd, name);
    return SUCCEEDED(hres);
  }

  auto is_valid() const -> bool {
    return _handle != nullptr && _handle != INVALID_HANDLE_VALUE;
  }

  auto join() -> bool {
    if (_handle == nullptr) return true;
    const auto ret = ::WaitForSingleObject(_handle, INFINITE);
    return ret == WAIT_OBJECT_0;
  }

  auto detach() -> bool {
    if (_handle == nullptr) return true;
    return ::CloseHandle(_handle);
  }
};

inline auto sleep_ms(unsigned millis) -> bool {
  if (millis <= 1) {
    return ::SwitchToThread();
  }
  ::Sleep(millis);
  return true;
}

}  // namespace sfc::sys::windows
