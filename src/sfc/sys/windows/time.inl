#pragma once

#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::windows {

static constexpr auto NANOS_PER_SEC = 1000000000UL;
static constexpr auto TICKS_PER_SECS = 10000000UL;  // 1 tick = 100ns

struct Instant {
  LONGLONG cnt;

  static auto freq() -> LONGLONG {
    auto val = ::LARGE_INTEGER{};
    if (!::QueryPerformanceFrequency(&val)) {
      return 1;
    }
    return val.QuadPart;
  }

  static auto now() noexcept -> Instant {
    auto cnt = ::LARGE_INTEGER{};
    if (!::QueryPerformanceCounter(&cnt)) {
      return Instant{0};
    }
    return Instant{cnt.QuadPart};
  }

  auto nanos() const noexcept -> ULONG64 {
    static const auto freq = Instant::freq();
    const auto nanos = cnt * NANOS_PER_SEC / freq;
    return static_cast<ULONG64>(nanos);
  }
};

struct SystemTime {
  ULONG64 cnt;

  static auto now() noexcept -> SystemTime {
    auto file_time = FILETIME{};
    ::GetSystemTimePreciseAsFileTime(&file_time);

    const auto cnt = static_cast<ULONG64>(file_time.dwHighDateTime) << 32 |
                     static_cast<ULONG64>(file_time.dwLowDateTime);

    return SystemTime{cnt};
  }

  auto nanos() const noexcept -> ULONG64 {
    static constexpr auto TICKS_PER_NANOS = 100UL;
    const auto nanos = cnt * TICKS_PER_NANOS;
    return nanos;
  }
};

struct DateTime {
  unsigned short year = 0;
  unsigned short month = 0;
  unsigned short day = 0;
  unsigned short hour = 0;
  unsigned short minute = 0;
  unsigned short second = 0;

  static inline auto to_filetime(ULONG64 secs) -> FILETIME {
    const auto cnts = secs * TICKS_PER_SECS;

    auto res = FILETIME{};
    res.dwLowDateTime = static_cast<DWORD>(cnts & 0xFFFFFFFFULL);
    res.dwHighDateTime = static_cast<DWORD>(cnts >> 32);
    return res;
  }

  static auto from_systemtime(const SYSTEMTIME& sys_time) -> DateTime {
    const auto res = DateTime{
        .year = static_cast<unsigned short>(sys_time.wYear),
        .month = static_cast<unsigned short>(sys_time.wMonth),
        .day = static_cast<unsigned short>(sys_time.wDay),
        .hour = static_cast<unsigned short>(sys_time.wHour),
        .minute = static_cast<unsigned short>(sys_time.wMinute),
        .second = static_cast<unsigned short>(sys_time.wSecond),
    };
    return res;
  }

  static auto from_utc(ULONG64 secs) -> DateTime {
    const auto utc_time = to_filetime(secs);

    auto sys_time = SYSTEMTIME{};
    ::FileTimeToSystemTime(&utc_time, &sys_time);

    return DateTime::from_systemtime(sys_time);
  }

  static auto from_local(ULONG64 secs) -> DateTime {
    const auto utc_time = to_filetime(secs);

    auto local_time = FILETIME{};
    ::FileTimeToLocalFileTime(&utc_time, &local_time);

    auto sys_time = SYSTEMTIME{};
    ::FileTimeToSystemTime(&local_time, &sys_time);

    return DateTime::from_systemtime(sys_time);
  }
};

}  // namespace sfc::sys::windows
