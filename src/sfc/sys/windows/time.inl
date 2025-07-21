#pragma once

#include <windows.h>

#include "sfc/core/mod.h"

namespace sfc::sys::time {

using instant_t = ULONG64;
using system_t = FILETIME;

static constexpr auto NANOS_PER_SEC = 1000000000UL;

inline auto instant_freq() -> instant_t {
  auto val = ::LARGE_INTEGER{};
  if (!::QueryPerformanceFrequency(&val)) {
    return 1;
  }
  return val.QuadPart;
}

inline auto instant_now() -> ULONG64 {
  static auto freq = instant_freq();

  auto cnt = ::LARGE_INTEGER{};
  if (!::QueryPerformanceCounter(&cnt)) {
    return 0;
  }
  const auto nanos = cnt.QuadPart * NANOS_PER_SEC / freq;
  return nanos;
}

inline auto system_now() -> ULONG64 {
  auto file_time = FILETIME{};
  ::GetSystemTimeAsFileTime(&file_time);

  const auto cnt = static_cast<ULONG64>(file_time.dwHighDateTime) +
                   (static_cast<ULONG64>(file_time.dwLowDateTime) << 32);

  const auto nanos = cnt * 100UL;
  return nanos;
}

struct DateTime {
  unsigned short year = 0;
  unsigned short month = 0;
  unsigned short mday = 0;
  unsigned short hour = 0;
  unsigned short min = 0;
  unsigned short sec = 0;

 public:
  static auto from_secs(ULONG64 secs) -> DateTime {
    // Convert from filetime_sec to FILETIME struct
    const auto cnts = secs * 10000UL;
    auto file_time = FILETIME{};
    file_time.dwLowDateTime = static_cast<DWORD>(cnts & 0xFFFFFFFFU);
    file_time.dwHighDateTime = static_cast<DWORD>(cnts >> 32);

    // Convert FILETIME to SYSTEMTIME
    auto sys_time = SYSTEMTIME{};
    FileTimeToSystemTime(&file_time, &sys_time);

    // Convert SYSTEMTIME to DateTime
    const auto res = DateTime{
        .year = sys_time.wYear,
        .month = sys_time.wMonth,
        .mday = sys_time.wDay,
        .hour = sys_time.wHour,
        .min = sys_time.wMinute,
        .sec = sys_time.wSecond,
    };
    return res;
  }
};

}  // namespace sfc::sys::time
