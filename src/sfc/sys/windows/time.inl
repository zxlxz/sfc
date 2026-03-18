#pragma once

#include "sfc/sys/windows/mod.inl"

namespace sfc::sys::windows {

static constexpr auto NANOS_PER_SEC = 1000000000UL;

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

    const auto q = cnt / freq;
    const auto r = cnt % freq;
    const auto nanos = q * NANOS_PER_SEC + r * NANOS_PER_SEC / freq;
    return static_cast<ULONG64>(nanos);
  }
};

struct SystemTime {
  static constexpr auto TICKS_PER_MICROS = 10UL;
  FILETIME t;

  static auto now() noexcept -> SystemTime {
    auto file_time = ::FILETIME{};
    ::GetSystemTimePreciseAsFileTime(&file_time);
    return SystemTime{file_time};
  }

  static auto from_micros(ULONG64 micros) -> SystemTime {
    const auto intervals = micros * TICKS_PER_MICROS;
    const auto file_time = FILETIME{
        .dwLowDateTime = static_cast<u32>(intervals),
        .dwHighDateTime = static_cast<u32>(intervals >> 32),
    };
    return SystemTime{file_time};
  }

  auto as_micros() const noexcept -> ULONG64 {
    const auto cnt = static_cast<ULONG64>(t.dwHighDateTime) << 32 | static_cast<ULONG64>(t.dwLowDateTime);
    return cnt / TICKS_PER_MICROS;
  }
};

struct DateTime {
  unsigned short year;
  unsigned short month;
  unsigned short day;
  unsigned short hour;
  unsigned short minute;
  unsigned short second;
  unsigned short millis;

 public:
  static auto from_filetime(const FILETIME& file_time) -> DateTime {
    auto sys_time = SYSTEMTIME{};
    ::FileTimeToSystemTime(&file_time, &sys_time);

    return DateTime{
        sys_time.wYear,
        sys_time.wMonth,
        sys_time.wDay,
        sys_time.wHour,
        sys_time.wMinute,
        sys_time.wSecond,
        sys_time.wMilliseconds,
    };
  }

  static auto from_utc(const SystemTime& sys_time) -> DateTime {
    return DateTime::from_filetime(sys_time.t);
  }

  static auto from_local(const SystemTime& sys_time) -> DateTime {
    auto local_time = FILETIME{};
    ::FileTimeToLocalFileTime(&sys_time.t, &local_time);
    return DateTime::from_filetime(local_time);
  }
};

}  // namespace sfc::sys::windows
