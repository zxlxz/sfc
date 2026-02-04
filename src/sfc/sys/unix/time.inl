#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::time {

static constexpr auto NANOS_PER_SEC = 1000000000UL;

using tm_t = struct tm;
using timespec_t = struct timespec;

static inline auto instant_now() -> unsigned long {
  auto ts = timespec_t{};
  ::clock_gettime(CLOCK_MONOTONIC, &ts);
  return ts.tv_sec * NANOS_PER_SEC + ts.tv_nsec;
}

static inline auto system_now() -> unsigned long {
  auto ts = timespec_t{};
  ::clock_gettime(CLOCK_REALTIME, &ts);
  return ts.tv_sec * NANOS_PER_SEC + ts.tv_nsec;
}

static inline auto make_datetime(const tm_t& tm, auto& dst) {
  dst.year = static_cast<unsigned short>(tm.tm_year + 1900);
  dst.month = static_cast<unsigned short>(tm.tm_mon + 1);
  dst.day = static_cast<unsigned short>(tm.tm_mday);
  dst.hour = static_cast<unsigned short>(tm.tm_hour);
  dst.minute = static_cast<unsigned short>(tm.tm_min);
  dst.second = static_cast<unsigned short>(tm.tm_sec);
}

static inline void make_utc(time_t secs, auto& dst) {
  auto tm = tm_t{};
  ::gmtime_r(&secs, &tm);
  make_datetime(tm, dst);
}

static inline void make_local(time_t secs, auto& dst) {
  auto tm = tm_t{};
  ::localtime_r(&secs, &tm);
  make_datetime(tm, dst);
}

}  // namespace sfc::sys::.time
