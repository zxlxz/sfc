#pragma once
#if defined(__unix__) || defined(__APPLE__)

#include <time.h>

#ifdef __APPLE__
// no need include, just make clang-tidy happy
#include <_time.h>
#endif

namespace sfc::sys::time {

static constexpr auto NANOS_PER_SEC = 1000000000UL;

using tm_t = struct tm;
using timespec_t = struct timespec;

inline auto instant_now() -> unsigned long {
  auto ts = timespec_t{};
  ::clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * NANOS_PER_SEC + ts.tv_nsec;
}

inline auto system_now() -> unsigned long {
  auto ts = timespec_t{};
  ::clock_gettime(CLOCK_REALTIME, &ts);
  return ts.tv_sec * NANOS_PER_SEC + ts.tv_nsec;
}

template <class T>
static inline auto make_utc(time_t secs) -> T {
  auto tm = tm_t{};
  ::gmtime_r(&secs, &tm);

  return T{
      static_cast<unsigned short>(tm.tm_year + 1900),
      static_cast<unsigned short>(tm.tm_mon + 1),
      static_cast<unsigned short>(tm.tm_mday),
      static_cast<unsigned short>(tm.tm_hour),
      static_cast<unsigned short>(tm.tm_min),
      static_cast<unsigned short>(tm.tm_sec),
  };
}

template <class T>
static inline auto make_local(time_t secs) -> T {
  auto tm = tm_t{};
  ::localtime_r(&secs, &tm);

  return T{
      static_cast<unsigned short>(tm.tm_year + 1900),
      static_cast<unsigned short>(tm.tm_mon + 1),
      static_cast<unsigned short>(tm.tm_mday),
      static_cast<unsigned short>(tm.tm_hour),
      static_cast<unsigned short>(tm.tm_min),
      static_cast<unsigned short>(tm.tm_sec),
  };
}

}  // namespace sfc::sys::time
#endif
