#pragma once

#include <Windows.h>

namespace sfc::sys::thread {

using thrd_t = HANDLE;
using thrd_ret_t = DWORD;

template <int N>
static auto wchar_to_u8(const wchar_t src[], char (&dst)[N]) -> int {
  return ::WideCharToMultiByte(CP_UTF8, 0, src, -1, dst, sizeof(dst), nullptr, nullptr);
}

template <int N>
static auto u8_to_wchar(const char src[], wchar_t (&dst)[N]) -> int {
  return ::MultiByteToWideChar(CP_UTF8, 0, src, -1, dst, sizeof(dst));
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

static inline void sleep_ns(DWORD64 nanos) {
  static constexpr auto NANOS_PER_MILLI = 1000000U;

  const auto millis = static_cast<DWORD>(nanos / NANOS_PER_MILLI);
  ::Sleep(millis);
}

}  // namespace sfc::sys::thread
