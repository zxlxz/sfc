#pragma once

#include <sys/time.h>
#include <time.h>

namespace sfc::sys::time {

struct Instant {
  time_t secs;
  long nanos;

  static auto now() -> Instant {
    struct timespec ts;
    ::clock_gettime(CLOCK_MONOTONIC, &ts);
    return Instant{ts.tv_sec, ts.tv_nsec};
  }
};

struct System {
  time_t secs;
  suseconds_t nanos;

  static auto now() -> System {
    struct timeval tv;
    ::gettimeofday(&tv, nullptr);
    return System{tv.tv_sec, 1000 * tv.tv_usec};
  }
};

struct DateTime {
  unsigned year;
  unsigned month;
  unsigned mday;
  unsigned hour;
  unsigned min;
  unsigned sec;

  static auto from_secs(time_t sec) -> DateTime {
    struct tm tm;
    ::localtime_r(&sec, &tm);

    const auto res = DateTime{
        static_cast<unsigned>(tm.tm_year + 1900), static_cast<unsigned>(tm.tm_mon + 1),
        static_cast<unsigned>(tm.tm_mday),        static_cast<unsigned>(tm.tm_hour),
        static_cast<unsigned>(tm.tm_min),         static_cast<unsigned>(tm.tm_sec),
    };
    return res;
  }
};

}  // namespace sfc::sys::time
