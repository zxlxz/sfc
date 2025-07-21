#pragma once

#include <sys/time.h>
#include <time.h>

namespace sfc::sys::time {

static constexpr auto NANOS_PER_SEC = 1000000000UL;

using tm_t = struct ::tm;
using timespec_t = struct ::timespec;

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

struct DateTime {
  unsigned short year;
  unsigned short month;
  unsigned short mday;
  unsigned short hour;
  unsigned short min;
  unsigned short sec;

 public:
  static auto from_secs(time_t secs) -> DateTime {
    auto tm = tm_t{};
    ::localtime_r(&secs, &tm);

    const auto res = DateTime{
        static_cast<unsigned short>(tm.tm_year + 1900),
        static_cast<unsigned short>(tm.tm_mon + 1),
        static_cast<unsigned short>(tm.tm_mday),
        static_cast<unsigned short>(tm.tm_hour),
        static_cast<unsigned short>(tm.tm_min),
        static_cast<unsigned short>(tm.tm_sec),
    };
    return res;
  }
};

}  // namespace sfc::sys::time
