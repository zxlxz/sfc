#pragma once

#include <windows.h>

#include "sfc/core/mod.h"

namespace sfc::sys::time {

struct Instant {
  // Windows performance counter is in counts since system boot
  LONG64 _cnt = 0;

 public:
  static auto performance_freq() -> LONG64 {
    auto val = ::LARGE_INTEGER{};
    if (!::QueryPerformanceFrequency(&val)) {
      return 1;
    }
    return val.QuadPart;
  }

  static auto now() -> Instant {
    auto val = ::LARGE_INTEGER{};
    if (!::QueryPerformanceCounter(&val)) {
      return Instant{};
    };
    return Instant{val.QuadPart};
  }

  auto nanos() const -> ULONG64 {
    static const auto freq = performance_freq();
    return static_cast<ULONG64>(_cnt * 1000000000U / freq);
  }
};

struct System {
  // Windows FILETIME is in 100-nanosecond intervals since January 1, 1601
  ::FILETIME _ft = {};

 public:
  static auto now() -> System {
    auto file_time = FILETIME{0};
    ::GetSystemTimeAsFileTime(&file_time);
    return {file_time};
  }

  static auto from_secs(ULONG64 secs) -> System {
    // Convert seconds to 100-nanosecond intervals
    const auto counts = secs * 10000000;

    const auto file_time = ::FILETIME{
        .dwLowDateTime = static_cast<DWORD>(counts & 0xFFFFFFFFU),
        .dwHighDateTime = static_cast<DWORD>(counts >> 32),
    };

    return System{file_time};
  }

  auto micros() const -> ULONG64 {
    // Convert FILETIME to 100-nanosecond intervals
    const auto counts = ULONG64(_ft.dwHighDateTime) + (ULONG64(_ft.dwLowDateTime) << 32);
    const auto micros = counts / 10;
    return micros;
  }
};

struct DateTime {
  USHORT year = 0;
  USHORT month = 0;
  USHORT mday = 0;
  USHORT hour = 0;
  USHORT min = 0;
  USHORT sec = 0;

 public:
  static auto from_systime(const System& sys_time) -> DateTime {
    // Convert FILETIME to SYSTEMTIME
    auto date_time = ::SYSTEMTIME{};
    if (!::FileTimeToSystemTime(&sys_time._ft, &date_time)) {
      return {};
    }

    // Convert SYSTEMTIME to DateTime
    const auto res = DateTime{
        date_time.wYear,
        date_time.wMonth,
        date_time.wDay,
        date_time.wHour,
        date_time.wMinute,
        date_time.wSecond,
    };
    return res;
  }
};

}  // namespace sfc::sys::time
