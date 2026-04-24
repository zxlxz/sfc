#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::unix {

static constexpr auto NANOS_PER_SEC = 1000000000UL;
static constexpr auto MICROS_PER_SEC = 1000000U;
static constexpr auto NANOS_PER_MICRO = 1000U;

struct Instant {
  timespec t = {};

  static auto now() noexcept -> Instant {
    auto ts = timespec{};
    (void)::clock_gettime(CLOCK_MONOTONIC, &ts);
    return Instant{ts};
  }

  auto nanos() const -> uint64_t {
    const auto nanos = t.tv_sec * NANOS_PER_SEC + t.tv_nsec;
    return static_cast<uint64_t>(nanos);
  }
};

struct SystemTime {
  struct timespec t = {};

  static auto now() noexcept -> SystemTime {
    struct timespec ts{};
    (void)::clock_gettime(CLOCK_REALTIME, &ts);
    return SystemTime{ts};
  }

  static auto from_micros(uint64_t micros) -> SystemTime {
    const auto secs = static_cast<time_t>(micros / MICROS_PER_SEC);
    const auto nsec = static_cast<uint32_t>(micros % MICROS_PER_SEC * NANOS_PER_MICRO);
    return SystemTime{{.tv_sec = secs, .tv_nsec = nsec}};
  }

  auto as_micros() const -> uint64_t {
    const auto nanos = t.tv_sec * MICROS_PER_SEC + t.tv_nsec / NANOS_PER_MICRO;
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

  static auto from_tm(const struct tm& t) -> DateTime {
    const auto res = DateTime{
        .year = static_cast<unsigned short>(t.tm_year + 1900),
        .month = static_cast<unsigned short>(t.tm_mon + 1),
        .day = static_cast<unsigned short>(t.tm_mday),
        .hour = static_cast<unsigned short>(t.tm_hour),
        .minute = static_cast<unsigned short>(t.tm_min),
        .second = static_cast<unsigned short>(t.tm_sec),
    };
    return res;
  }

  static auto from_utc(const SystemTime& sys_time) -> DateTime {
    struct tm tm{};
    ::gmtime_r(&sys_time.t.tv_sec, &tm);
    return DateTime::from_tm(tm);
  }

  static auto from_local(const SystemTime& sys_time) -> DateTime {
    struct tm tm{};
    ::localtime_r(&sys_time.t.tv_sec, &tm);
    return DateTime::from_tm(tm);
  }
};

}  // namespace sfc::sys::unix
