#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/time.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/time.inl"
#endif

#include "sfc/time/datetime.h"

namespace sfc::time {

static auto make_datetime(auto sys_date_time, auto micros) -> DateTime {
  const auto res = DateTime{
      .year = static_cast<u16>(sys_date_time.year),
      .month = static_cast<u16>(sys_date_time.month),
      .day = static_cast<u16>(sys_date_time.day),
      .hour = static_cast<u16>(sys_date_time.hour),
      .minute = static_cast<u16>(sys_date_time.minute),
      .second = static_cast<u16>(sys_date_time.second),
      .micros = micros,
  };
  return res;
};

auto DateTime::from_utc(SystemTime t) noexcept -> DateTime {
  const auto sys_time = sys::SystemTime::from_micros(t._micros);
  const auto sys_date = sys::DateTime::from_utc(sys_time);
  return time::make_datetime(sys_date, t.subsec_micros());
}

auto DateTime::from_local(SystemTime t) noexcept -> DateTime {
  const auto sys_time = sys::SystemTime::from_micros(t._micros);
  const auto sys_date = sys::DateTime::from_local(sys_time);
  return time::make_datetime(sys_date, t.subsec_micros());
}

}  // namespace sfc::time
