#pragma once

#include <windows.h>

#include "sfc/core/mod.h"

namespace sfc::sys::time {

struct Instant {
  LONG64 _cnt;

  static auto formance_freq() -> LONG64 {
    auto val = ::LARGE_INTEGER{};
    if (::QueryPerformanceFrequency(&val) == 0) {
      return 1;
    }
    return val.QuadPart;
  }

  static auto now() -> Instant {
    auto cnt = ::LARGE_INTEGER{};
    if (::QueryPerformanceCounter(&cnt) == 0) {
      return {0};
    };
    return {cnt.QuadPart};
  }

  auto nanos() const -> ULONG64 {
    static const auto freq = formance_freq();
    return static_cast<ULONG64>(_cnt * 1000000000 / freq);
  }
};

struct System {
  ::FILETIME _ft;

  static auto now() -> System {
    auto file_time = FILETIME{0};
    ::GetSystemTimeAsFileTime(&file_time);
    return {file_time};
  }

  auto micros() const -> ULONG64 {
    const auto cnt = ULONG64(_ft.dwHighDateTime) + (ULONG64(_ft.dwLowDateTime) << 32);
    return cnt / 10;
  }
};

struct DateTime {
  USHORT year;
  USHORT month;
  USHORT mday;
  USHORT hour;
  USHORT min;
  USHORT sec;

  static auto from_secs(ULONG64 secs) -> DateTime {
    const auto ft_cnt = secs * 10000000;

    auto file_time           = ::FILETIME{};
    file_time.dwLowDateTime  = static_cast<DWORD>(ft_cnt & 0xFFFFFFFFU);
    file_time.dwHighDateTime = static_cast<DWORD>(ft_cnt >> 32);

    auto sys_time = ::SYSTEMTIME{};
    if (::FileTimeToSystemTime(&file_time, &sys_time) == 0) {
      return {};
    }

    const auto res = DateTime{
        sys_time.wYear,
        sys_time.wMonth,
        sys_time.wDay,
        sys_time.wHour,
        sys_time.wMinute,
        sys_time.wSecond,
    };

    return res;
  }
};

}  // namespace sfc::sys::time
