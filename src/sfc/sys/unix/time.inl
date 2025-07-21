#pragma once

#include <sys/time.h>
#include <time.h>

namespace sfc::sys::time {

using timespec_t = struct ::timespec;

struct Instant {
  time_t _nanos;

  static auto now() -> Instant {
    auto ts = timespec_t{};
    ::clock_gettime(CLOCK_MONOTONIC, &ts);

    const auto nanos = ts.tv_sec * 1000000000 + ts.tv_nsec;
    return Instant{nanos};
  }

  auto nanos() const -> time_t {
    return _nanos;
  }
};

struct System {
  time_t _nanos;

 public:
  static auto from_secs(time_t s) -> System {
    // 1s = 1000000000ns
    return System{s * 1000000000};
  }

  static auto now() -> System {
    auto ts = timespec_t{};
    ::clock_gettime(CLOCK_REALTIME, &ts);

    const auto nanos = time_t(ts.tv_sec * 1000000000 + ts.tv_nsec);
    return System{nanos};
  }

  auto micros() const -> time_t {
    return _nanos / 1000;
  }
};

struct DateTime {
  unsigned short year;
  unsigned short month;
  unsigned short mday;
  unsigned short hour;
  unsigned short min;
  unsigned short sec;

 public:
  static auto from_secs(time_t secs) -> DateTime {
    struct tm tm{};
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
