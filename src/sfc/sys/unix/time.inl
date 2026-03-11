#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::unix {

static constexpr auto NANOS_PER_SEC = 1000000000UL;

using tm_t = struct ::tm;
using timespec_t = struct ::timespec;

struct Instant {
  timespec_t _ts = {};

  static auto now() noexcept -> Instant {
    auto ts = timespec_t{};
    ::clock_gettime(CLOCK_MONOTONIC, &ts);
    return Instant{ts};
  }

  auto nanos() const -> uint64_t {
    const auto nanos = _ts.tv_sec * NANOS_PER_SEC + _ts.tv_nsec;
    return static_cast<uint64_t>(nanos);
  }
};

struct SystemTime {
  timespec_t _ts = {};

  static auto now() noexcept -> SystemTime {
    auto ts = timespec_t{};
    ::clock_gettime(CLOCK_REALTIME, &ts);
    return SystemTime{ts};
  }

  auto nanos() const -> uint64_t {
    const auto nanos = _ts.tv_sec * NANOS_PER_SEC + _ts.tv_nsec;
    return static_cast<uint64_t>(nanos);
  }
};

struct DateTime {
  unsigned short year = 0;
  unsigned short month = 0;
  unsigned short day = 0;
  unsigned short hour = 0;
  unsigned short minute = 0;
  unsigned short second = 0;

  static auto from_tm(const tm_t& tm) -> DateTime {
    const auto res = DateTime{
        .year = static_cast<unsigned short>(tm.tm_year + 1900),
        .month = static_cast<unsigned short>(tm.tm_mon + 1),
        .day = static_cast<unsigned short>(tm.tm_mday),
        .hour = static_cast<unsigned short>(tm.tm_hour),
        .minute = static_cast<unsigned short>(tm.tm_min),
        .second = static_cast<unsigned short>(tm.tm_sec),
    };
    return res;
  }

  static auto from_utc(time_t srcs) -> DateTime {
    auto tm = tm_t{};
    ::gmtime_r(&srcs, &tm);
    return from_tm(tm);
  }

  static auto from_local(time_t srcs) -> DateTime {
    auto tm = tm_t{};
    ::localtime_r(&srcs, &tm);
    return from_tm(tm);
  }
};

}  // namespace sfc::sys::unix
