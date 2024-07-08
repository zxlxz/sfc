#pragma once

#include <sys/time.h>
#include <time.h>

namespace sfc::sys::time {

struct Instant {
  time_t secs;
  time_t nanos;

  static auto now() -> Instant {
    struct timespec ts;
    ::clock_gettime(CLOCK_MONOTONIC, &ts);
    return {ts.tv_sec, ts.tv_nsec};
  }
};

struct DateTime {
  unsigned year;
  unsigned month;
  unsigned mday;
  unsigned hour;
  unsigned min;
  unsigned sec;
  unsigned usec;

  static auto now() -> DateTime {
    struct timeval tv;
    ::gettimeofday(&tv, nullptr);

    struct tm tm;
    ::localtime_r(&tv.tv_sec, &tm);

    const auto res = DateTime{
        static_cast<unsigned>(tm.tm_year + 1990), static_cast<unsigned>(tm.tm_mon + 1),
        static_cast<unsigned>(tm.tm_mday),        static_cast<unsigned>(tm.tm_hour),
        static_cast<unsigned>(tm.tm_min),         static_cast<unsigned>(tm.tm_sec),
        static_cast<unsigned>(tv.tv_usec),
    };
    return res;
  }
};

}  // namespace sfc::sys::time
