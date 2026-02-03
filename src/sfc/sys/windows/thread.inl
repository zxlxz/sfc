#pragma once
#ifdef _WIN32
#include <Windows.h>
#include <process.h>
#include <timeapi.h>

namespace sfc::sys::thread {

using ret_t = unsigned;
using tid_t = DWORD;
using thrd_t = HANDLE;

template <int N>
auto wchar_to_u8(const wchar_t src[], char (&dst)[N]) -> int {
  const auto write_bytes = ::WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, N, nullptr, nullptr);
}

template <int N>
static inline auto to_os_str(const char src[], wchar_t (&dst)[N]) -> UINT {
  const auto ret = ::MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, N);
  return ret > 0 ? static_cast<UINT>(ret - 1) : 0;
}

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
inline auto thrd_name(thrd_t thr, char (&buf)[N]) -> const char* {
  wchar_t* wbuf = nullptr;
  if (FAILED(::GetThreadDescription(thr, &wbuf))) {
    return nullptr;
  };

  wchar_to_u8(wbuf, buf);
  ::LocalFree(wbuf);

  return buf;
}

inline auto thrd_setname(const char* name) -> bool {
  wchar_t os_name[MAX_PATH];
  if (!to_os_str(name, os_name)) {
    return false;
  }

  const auto thrd = ::GetCurrentThread();
  const auto hres = ::SetThreadDescription(thrd, os_name);
  return SUCCEEDED(hres);
}

inline void thrd_sleep_ms(DWORD millis) {
  if (millis < 2) {
    ::SwitchToThread();
    return;
  }
  ::Sleep(millis);
}

}  // namespace sfc::sys::thread
#endif
