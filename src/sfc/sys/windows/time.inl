#pragma once
#ifdef _WIN32
#include <Windows.h>

namespace sfc::sys::time {

static constexpr auto NANOS_PER_SEC = 1000000000UL;

inline auto instant_freq() -> ULONG64 {
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

  const auto cnt = static_cast<ULONG64>(file_time.dwHighDateTime) << 32 |
                   static_cast<ULONG64>(file_time.dwLowDateTime);

  const auto nanos = cnt * 100UL;
  return nanos;
}

template <class T>
static inline auto make_utc(ULONG64 secs) -> T {
  // Convert from seconds to 100-nanosecond intervals for FILETIME
  const auto cnts = secs * 10000000UL;
  auto utc_time = FILETIME{};
  utc_time.dwLowDateTime = static_cast<DWORD>(cnts & 0xFFFFFFFFU);
  utc_time.dwHighDateTime = static_cast<DWORD>(cnts >> 32);

  auto date_time = SYSTEMTIME{};
  FileTimeToSystemTime(&utc_time, &date_time);

  return T{
      static_cast<unsigned short>(date_time.wYear),
      static_cast<unsigned short>(date_time.wMonth),
      static_cast<unsigned short>(date_time.wDay),
      static_cast<unsigned short>(date_time.wHour),
      static_cast<unsigned short>(date_time.wMinute),
      static_cast<unsigned short>(date_time.wSecond),
  };
}

template <class T>
static inline auto make_local(ULONG64 secs) -> T {
  // Convert from seconds to 100-nanosecond intervals for FILETIME
  const auto cnts = secs * 10000000UL;
  auto utc_time = FILETIME{};
  utc_time.dwLowDateTime = static_cast<DWORD>(cnts & 0xFFFFFFFFU);
  utc_time.dwHighDateTime = static_cast<DWORD>(cnts >> 32);

  auto local_time = FILETIME{};
  FileTimeToLocalFileTime(&utc_time, &local_time);

  auto date_time = SYSTEMTIME{};
  FileTimeToSystemTime(&local_time, &date_time);

  return T{
      static_cast<unsigned short>(date_time.wYear),
      static_cast<unsigned short>(date_time.wMonth),
      static_cast<unsigned short>(date_time.wDay),
      static_cast<unsigned short>(date_time.wHour),
      static_cast<unsigned short>(date_time.wMinute),
      static_cast<unsigned short>(date_time.wSecond),
  };
}

}  // namespace sfc::sys::time
#endif
