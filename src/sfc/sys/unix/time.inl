#pragma once

#include <sys/time.h>
#include <time.h>

namespace sfc::sys::time {

struct Instant {
  time_t _nanos;

  static auto now() -> Instant {
    struct timespec ts;
    ::clock_gettime(CLOCK_MONOTONIC, &ts);

    const auto nanos = ts.tv_sec * 1000000000 + ts.tv_nsec;
    return Instant{nanos};
  }

  auto nanos() const -> __uint64_t {
    return static_cast<__uint64_t>(_nanos);
  }
};

struct System {
  __uint64_t _nanos;

 public:
  static auto from_secs(__uint64_t s) -> System {
    // 1s = 1000000000ns
    return System{s * 1000000000};
  }

  static auto now() -> System {
    struct timespec ts;
    ::clock_gettime(CLOCK_REALTIME, &ts);

    const auto nanos = __uint64_t(ts.tv_sec * 1000000000 + ts.tv_nsec);
    return System{nanos};
  }

  auto micros() const -> __uint64_t {
    return static_cast<__uint64_t>(_nanos) / 1000;
  }
};

struct DateTime {
  __uint16_t year;
  __uint16_t month;
  __uint16_t mday;
  __uint16_t hour;
  __uint16_t min;
  __uint16_t sec;

 public:
  static auto from_secs(__uint64_t secs_u64) -> DateTime {
    const auto secs = time_t(secs_u64);

    struct tm tm{};
    ::localtime_r(&secs, &tm);

    const auto res = DateTime{
        static_cast<__uint16_t>(tm.tm_year + 1900),
        static_cast<__uint16_t>(tm.tm_mon + 1),
        static_cast<__uint16_t>(tm.tm_mday),
        static_cast<__uint16_t>(tm.tm_hour),
        static_cast<__uint16_t>(tm.tm_min),
        static_cast<__uint16_t>(tm.tm_sec),
    };
    return res;
  }
};

}  // namespace sfc::sys::time
