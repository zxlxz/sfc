#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/time.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/time.inl"
#endif

#include "sfc/time/datetime.h"

namespace sfc::time {

auto DateTime::from_utc(SystemTime t) noexcept -> DateTime {
  auto sys_time = sys::SystemTime::from_micros(t._micros);
  auto sys_date = sys::DateTime::from_utc(sys_time);

  const auto res = DateTime{
      .year = sys_date.year,
      .month = sys_date.month,
      .day = sys_date.day,
      .hour = sys_date.hour,
      .minute = sys_date.minute,
      .second = sys_date.second,
      .micros = t.subsec_micros(),
  };
  return res;
}

auto DateTime::from_local(SystemTime t) noexcept -> DateTime {
  auto sys_time = sys::SystemTime::from_micros(t._micros);
  auto sys_date = sys::DateTime::from_local(sys_time);

  const auto res = DateTime{
      .year = sys_date.year,
      .month = sys_date.month,
      .day = sys_date.day,
      .hour = sys_date.hour,
      .minute = sys_date.minute,
      .second = sys_date.second,
      .micros = t.subsec_micros(),
  };
  return res;
}

}  // namespace sfc::time
