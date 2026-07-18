#define _SFC_SYS_TIME_
#include "sfc/sys.h"
#include "sfc/time/datetime.h"

namespace sfc::time {

static auto make_datetime(const auto& t, auto micros) -> DateTime {
  const auto res = DateTime{
      .year = t.year,
      .month = t.month,
      .day = t.day,
      .hour = t.hour,
      .minute = t.minute,
      .second = t.second,
      .micros = micros,
  };
  return res;
};

auto DateTime::from_utc(SystemTime t) noexcept -> DateTime {
  const auto sys_time = sys::SystemTime{t._micros};
  const auto sys_date = sys::DateTime::from_utc(sys_time);
  return time::make_datetime(sys_date, t.subsec_micros());
}

auto DateTime::from_local(SystemTime t) noexcept -> DateTime {
  const auto sys_time = sys::SystemTime{t._micros};
  const auto sys_date = sys::DateTime::from_local(sys_time);
  return time::make_datetime(sys_date, t.subsec_micros());
}

void DateTime::fmt(fmt::Formatter& f) const {
  f.write_fmt("{04}-{02}-{02} {02}:{02}:{02}.{06}", year, month, day, hour, minute, second, micros);
}

}  // namespace sfc::time
