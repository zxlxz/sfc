#pragma once
#ifdef _WIN32
#include <Windows.h>

namespace sfc::sys::thread {

using ret_t = DWORD;
using tid_t = DWORD;
using thrd_t = HANDLE;

template <int N>
auto wchar_to_u8(const wchar_t src[], char (&dst)[N]) -> int {
  return ::WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, N, nullptr, nullptr);
}

template <int N>
auto u8_to_wchar(const char src[], wchar_t (&dst)[N]) -> int {
  return ::MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, N);
}

inline auto current() -> thrd_t {
  return ::GetCurrentThread();
}

inline auto start(SIZE_T stack_size, DWORD (*func)(void*), void* data) -> thrd_t {
  return ::CreateThread(nullptr, stack_size, func, data, 0, nullptr);
}

inline auto join(thrd_t thr) -> bool {
  const auto ret = ::WaitForSingleObject(thr, INFINITE);
  return ret == WAIT_OBJECT_0;
}

inline auto detach(thrd_t thr) -> bool {
  return ::CloseHandle(thr);
}

inline void yield() {
  ::SwitchToThread();
}

template <int N>
inline auto get_name(thrd_t thr, char (&buf)[N]) -> const char* {
  wchar_t* wbuf = nullptr;
  if (FAILED(::GetThreadDescription(thr, &wbuf))) {
    return nullptr;
  };

  wchar_to_u8(wbuf, buf);
  ::LocalFree(wbuf);

  return buf;
}

inline auto set_name(const char* name) -> bool {
  wchar_t wbuff[256];
  if (u8_to_wchar(name, wbuff) == 0) {
    return false;
  }

  const auto thrd = ::GetCurrentThread();
  const auto hres = ::SetThreadDescription(thrd, wbuff);
  return SUCCEEDED(hres);
}

inline void sleep_ms(DWORD millis) {
  ::Sleep(millis);
}

}  // namespace sfc::sys::thread
#endif
