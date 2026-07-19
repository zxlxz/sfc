#include <Windows.h>
#undef min
#undef max

#include "sfc/sys/windows/time.h"

namespace sfc::sys::windows {

static constexpr auto NANOS_PER_SEC = 1000000000UL;
static constexpr auto MICROS_PER_FILETIME_TICK = 10UL;

static auto query_freq() -> i64 {
  auto val = ::LARGE_INTEGER{};
  if (!::QueryPerformanceFrequency(&val)) {
    return 1;
  }
  return val.QuadPart;
}

auto Instant::now() noexcept -> Instant {
  static const auto freq = query_freq();

  auto cnt = ::LARGE_INTEGER{};
  if (!::QueryPerformanceCounter(&cnt)) {
    return {0, 0};
  }

  const auto q = cnt.QuadPart / freq;
  const auto r = cnt.QuadPart % freq;
  const auto sec = num::cast_unsigned(q);
  const auto nsec = num::cast_unsigned(r * i64(NANOS_PER_SEC) / freq);
  return {sec, nsec};
}

auto SystemTime::now() noexcept -> SystemTime {
  auto file_time = ::FILETIME{};
  ::GetSystemTimePreciseAsFileTime(&file_time);

  const auto cnt = (u64{file_time.dwHighDateTime} << 32) | u64{file_time.dwLowDateTime};
  return {cnt / MICROS_PER_FILETIME_TICK};
}

static auto to_datetime(const ::SYSTEMTIME& t) -> DateTime {
  const auto res = DateTime{
      .year = u16(t.wYear),
      .month = u16(t.wMonth),
      .day = u16(t.wDay),
      .hour = u16(t.wHour),
      .minute = u16(t.wMinute),
      .second = u16(t.wSecond),
  };
  return res;
}

static auto to_filetime(u64 micros) -> ::FILETIME {
  const auto intervals = micros * MICROS_PER_FILETIME_TICK;
  return ::FILETIME{
      .dwLowDateTime = u32(intervals & 0xFFFFFFFFU),
      .dwHighDateTime = u32(intervals >> 32),
  };
}

auto DateTime::from_utc(const SystemTime& sys_time) -> DateTime {
  const auto file_time = to_filetime(sys_time.micros);

  auto sys_time_out = ::SYSTEMTIME{};
  ::FileTimeToSystemTime(&file_time, &sys_time_out);
  return to_datetime(sys_time_out);
}

auto DateTime::from_local(const SystemTime& sys_time) -> DateTime {
  const auto file_time = to_filetime(sys_time.micros);

  auto local_time = ::FILETIME{};
  ::FileTimeToLocalFileTime(&file_time, &local_time);

  auto sys_time_out = ::SYSTEMTIME{};
  ::FileTimeToSystemTime(&local_time, &sys_time_out);
  return to_datetime(sys_time_out);
}

}  // namespace sfc::sys::windows
