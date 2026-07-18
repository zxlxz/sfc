#include <time.h>
#include <sys/time.h>

#include "sfc/sys/posix/time.h"

namespace sfc::sys::posix {

static constexpr auto MICROS_PER_SEC = 1000000U;
static constexpr auto NANOS_PER_MICRO = 1000U;

auto Instant::now() noexcept -> Instant {
  auto ts = ::timespec{};
  (void)::clock_gettime(CLOCK_MONOTONIC, &ts);

  const auto secs = num::cast_unsigned(ts.tv_sec);
  const auto nsec = num::cast_unsigned(ts.tv_nsec);
  return Instant{secs, nsec};
}

auto SystemTime::now() noexcept -> SystemTime {
  auto ts = ::timespec{};
  (void)::clock_gettime(CLOCK_REALTIME, &ts);

  const auto secs = num::cast_unsigned(ts.tv_sec);
  const auto nsec = num::cast_unsigned(ts.tv_nsec);
  const auto micros = secs * MICROS_PER_SEC + nsec / NANOS_PER_MICRO;
  return SystemTime{micros};
}

static auto to_datetime(const ::tm& t) -> DateTime {
  const auto res = DateTime{
      .year = u16(t.tm_year + 1900),
      .month = u16(t.tm_mon + 1),
      .day = u16(t.tm_mday),
      .hour = u16(t.tm_hour),
      .minute = u16(t.tm_min),
      .second = u16(t.tm_sec),
  };
  return res;
}

auto DateTime::from_utc(const SystemTime& sys_time) -> DateTime {
  const auto t = time_t(sys_time.micros / MICROS_PER_SEC);
  auto tm = ::tm{};
  ::gmtime_r(&t, &tm);
  return to_datetime(tm);
}

auto DateTime::from_local(const SystemTime& sys_time) -> DateTime {
  const auto t = time_t(sys_time.micros / MICROS_PER_SEC);
  auto tm = ::tm{};
  ::localtime_r(&t, &tm);
  return to_datetime(tm);
}

}  // namespace sfc::sys::posix
