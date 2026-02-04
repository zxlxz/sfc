#pragma once
#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::thread {

using ret_t = unsigned;
using tid_t = DWORD;
using thrd_t = HANDLE;

inline auto thrd_current() -> thrd_t {
  return ::GetCurrentThread();
}

inline auto thrd_create(size_t stack_size, ret_t(__stdcall* func)(void*), void* data) -> thrd_t {
  unsigned thrd_id = 0;
  const auto handle = ::_beginthreadex(nullptr, static_cast<unsigned>(stack_size), func, data, 0, &thrd_id);
  return reinterpret_cast<HANDLE>(handle);
}

inline auto thrd_join(thrd_t thr) -> bool {
  if (thr == nullptr || thr == INVALID_HANDLE_VALUE) {
    return false;
  }
  const auto ret = ::WaitForSingleObject(thr, INFINITE);
  return ret == WAIT_OBJECT_0;
}

inline auto thrd_detach(thrd_t thr) -> bool {
  if (thr == nullptr || thr == INVALID_HANDLE_VALUE) {
    return false;
  }
  return ::CloseHandle(thr);
}

inline void thrd_yield() {
  ::SwitchToThread();
}

template <int N>
inline auto thrd_name(thrd_t thr, char (&u8_buf)[N]) -> const char* {
  wchar_t* os_buf = nullptr;
  if (FAILED(::GetThreadDescription(thr, &os_buf))) {
    return nullptr;
  };

  to_u8_str(os_buf, u8_buf);
  ::LocalFree(os_buf);

  return u8_buf;
}

inline auto thrd_setname(const wchar_t* name) -> bool {
  const auto thrd = ::GetCurrentThread();
  const auto hres = ::SetThreadDescription(thrd, name);
  return SUCCEEDED(hres);
}

inline void thrd_sleep_ms(DWORD millis) {
  if (millis < 2) {
    ::SwitchToThread();
    return;
  }
  ::Sleep(millis);
}

}  // namespace sfc::sys::.thread
