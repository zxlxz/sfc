#pragma once

#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::windows {

struct Thread {
  using ret_t = unsigned long;
  using func_t = ret_t (*)(void*);

  HANDLE _handle = nullptr;

 public:
  static auto current() -> Thread {
    return Thread{::GetCurrentThread()};
  }

  static auto spawn(size_t stack_size, func_t func, void* data) -> Thread {
    DWORD tid = 0;
    auto handle = ::CreateThread(nullptr, stack_size, func, data, 0, &tid);
    if (handle == nullptr) {
      return {};
    }
    return Thread{handle};
  }

  static auto yield() -> bool {
    const auto ret = ::SwitchToThread();
    return bool(ret);
  }

  static auto set_name(const wchar_t* name) -> bool {
    const auto hres = ::SetThreadDescription(::GetCurrentThread(), name);
    return SUCCEEDED(hres);
  }

  auto is_valid() const -> bool {
    return _handle != nullptr && _handle != INVALID_HANDLE_VALUE;
  }

  auto join() -> bool {
    if (_handle == nullptr || _handle == INVALID_HANDLE_VALUE) {
      return false;
    }
    const auto ret = ::WaitForSingleObject(_handle, INFINITE);
    return ret == WAIT_OBJECT_0;
  }

  auto detach() -> bool {
    if (_handle == nullptr || _handle == INVALID_HANDLE_VALUE) {
      return false;
    }
    return ::CloseHandle(_handle);
  }

  auto tid() const -> unsigned {
    if (_handle == nullptr || _handle == INVALID_HANDLE_VALUE) {
      return 0;
    }
    return static_cast<unsigned>(::GetThreadId(_handle));
  }
};

inline auto sleep_ms(unsigned millis) -> bool {
  if (millis < 2) {
    ::SwitchToThread();
    return true;
  }
  ::Sleep(millis);
  return true;
}

}  // namespace sfc::sys::windows
