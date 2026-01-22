#pragma once
#ifdef _WIN32
#include <Windows.h>

namespace sfc::sys::time {

static constexpr auto NANOS_PER_SEC = 1000000000UL;
static constexpr auto TICKS_PER_SECS = 10000000UL;  // 1 tick = 100ns

static inline auto instant_freq() -> ULONG64 {
  auto val = ::LARGE_INTEGER{};
  if (!::QueryPerformanceFrequency(&val)) {
    return 1;
  }
  return val.QuadPart;
}

static inline auto instant_now() -> ULONG64 {
  static auto freq = instant_freq();

  auto cnt = ::LARGE_INTEGER{};
  if (!::QueryPerformanceCounter(&cnt)) {
    return 0;
  }
  const auto nanos = cnt.QuadPart * NANOS_PER_SEC / freq;
  return nanos;
}

static inline auto system_now() -> ULONG64 {
  auto file_time = FILETIME{};
  ::GetSystemTimePreciseAsFileTime(&file_time);

  const auto cnt = static_cast<ULONG64>(file_time.dwHighDateTime) << 32 | static_cast<ULONG64>(file_time.dwLowDateTime);

  const auto nanos = cnt * 100UL;
  return nanos;
}

static inline auto make_filetime(ULONG64 secs) -> FILETIME {
  const auto cnts = secs * TICKS_PER_SECS;

  auto res = FILETIME{};
  res.dwLowDateTime = static_cast<DWORD>(cnts & 0xFFFFFFFFULL);
  res.dwHighDateTime = static_cast<DWORD>(cnts >> 32);
  return res;
}

static inline void make_datetime(const FILETIME& file_time, auto& dst) {
  auto sys_time = SYSTEMTIME{};
  ::FileTimeToSystemTime(&file_time, &sys_time);

  dst.year = static_cast<unsigned short>(sys_time.wYear);
  dst.month = static_cast<unsigned short>(sys_time.wMonth);
  dst.day = static_cast<unsigned short>(sys_time.wDay);
  dst.hour = static_cast<unsigned short>(sys_time.wHour);
  dst.minute = static_cast<unsigned short>(sys_time.wMinute);
  dst.second = static_cast<unsigned short>(sys_time.wSecond);
}

static inline void make_utc(ULONG64 secs, auto& dst) {
  const auto utc_time = make_filetime(secs);
  make_datetime(utc_time, dst);
}

static inline void make_local(ULONG64 secs, auto& dst) {
  const auto utc_time = make_filetime(secs);
  auto local_time = FILETIME{};
  ::FileTimeToLocalFileTime(&utc_time, &local_time);
  make_datetime(local_time, dst);
}

}  // namespace sfc::sys::time
#endif
