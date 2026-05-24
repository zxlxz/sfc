#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::unix {

static constexpr auto NANOS_PER_SEC = 1000000000UL;
static constexpr auto MICROS_PER_SEC = 1000000U;
static constexpr auto NANOS_PER_MICRO = 1000U;

struct Instant {
  ::timespec t = {};

  static auto now() noexcept -> Instant {
    auto ts = ::timespec{};
    (void)::clock_gettime(CLOCK_MONOTONIC, &ts);
    return Instant{ts};
  }

  auto nanos() const -> uint64_t {
    const auto secs = num::cast_unsigned(t.tv_sec);
    const auto nsec = num::cast_unsigned(t.tv_nsec);
    return secs * NANOS_PER_SEC + nsec;
  }
};

struct SystemTime {
  ::timespec t = {};

  static auto now() noexcept -> SystemTime {
    auto ts = ::timespec{};
    (void)::clock_gettime(CLOCK_REALTIME, &ts);
    return SystemTime{ts};
  }

  static auto from_micros(uint64_t micros) -> SystemTime {
    const auto secs = num::cast_signed(micros / MICROS_PER_SEC);
    const auto nsec = num::cast_signed(micros % MICROS_PER_SEC * NANOS_PER_MICRO);
    return SystemTime{{.tv_sec = secs, .tv_nsec = nsec}};
  }

  auto as_micros() const -> uint64_t {
    const auto secs = num::cast_unsigned(t.tv_sec);
    const auto nsec = num::cast_unsigned(t.tv_nsec);
    return secs * MICROS_PER_SEC + nsec / NANOS_PER_MICRO;
  }
};

struct DateTime {
  u16 year = 0;
  u16 month = 0;
  u16 day = 0;
  u16 hour = 0;
  u16 minute = 0;
  u16 second = 0;

  static auto from_tm(const struct tm& t) -> DateTime {
    const auto res = DateTime{
        .year   = num::saturating_cast<u16>(t.tm_year + 1900),
        .month  = num::saturating_cast<u16>(t.tm_mon + 1),
        .day    = num::saturating_cast<u16>(t.tm_mday),
        .hour   = num::saturating_cast<u16>(t.tm_hour),
        .minute = num::saturating_cast<u16>(t.tm_min),
        .second = num::saturating_cast<u16>(t.tm_sec),
    };
    return res;
  }

  static auto from_utc(const SystemTime& sys_time) -> DateTime {
    auto tm = ::tm{};
    ::gmtime_r(&sys_time.t.tv_sec, &tm);
    return DateTime::from_tm(tm);
  }

  static auto from_local(const SystemTime& sys_time) -> DateTime {
    auto tm = ::tm{};
    ::localtime_r(&sys_time.t.tv_sec, &tm);
    return DateTime::from_tm(tm);
  }
};

}  // namespace sfc::sys::unix
