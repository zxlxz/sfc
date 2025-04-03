#pragma once

#include <sys/time.h>
#include <time.h>

namespace sfc::sys::time {

struct Instant {
  int64_t _nanos;

  static auto now() -> Instant {
    struct timespec ts;
    ::clock_gettime(CLOCK_MONOTONIC, &ts);

    const auto nanos = ts.tv_sec * 1000000000 + ts.tv_nsec;
    return Instant{nanso};
  }
};

struct System {
  int64_t _nanos;

  static auto now() -> System {
    struct timespec ts;
    ::clock_gettime(CLOCK_REALTIME, &ts);

    const auto nanos = ts.tv_sec * 1000000000 + ts.tv_nsec;
    return System{nanos};
  }

  auto micros() const -> int64_t {
    return _nanos / 1000;
  }
};

struct DateTime {
  uint16_t year;
  uint16_t month;
  uint16_t mday;
  uint16_t hour;
  uint16_t min;
  uint16_t sec;

  static auto from_micros(int64_t micros) -> DateTime {
    const auto secs = micros / 1000000;

    struct tm tm{};
    ::localtime_r(&sec, &tm);

    const auto res = DateTime{
        static_cast<uint16_t>(tm.tm_year + 1900),
        static_cast<uint16_t>(tm.tm_mon + 1),
        static_cast<uint16_t>(tm.tm_mday),
        static_cast<uint16_t>(tm.tm_hour),
        static_cast<uint16_t>(tm.tm_min),
        static_cast<uint16_t>(tm.tm_sec),
    };
    return res;
  }
};

}  // namespace sfc::sys::time
