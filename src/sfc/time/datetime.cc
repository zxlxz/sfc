#include "datetime.h"

#include "sfc/sys/time.h"

namespace sfc::time {

namespace sys_imp = sys::time;

auto NaiveTime::from_hms(u32 hour, u32 min, u32 sec) -> NaiveTime {
  const auto secs = (hour * 60 + min) * 60 + sec;
  return {secs, 0};
}

auto NaiveTime::from_hms_micro(u32 hour, u32 min, u32 sec, u32 micros) -> NaiveTime {
  const auto secs = (hour * 60 + min) * 60 + sec;
  return {secs, micros};
}

auto NaiveTime::from_hms_milli(u32 hour, u32 min, u32 sec, u32 millis) -> NaiveTime {
  const auto total_secs = (hour * 60 + min) * 60 + sec;
  return {total_secs, static_cast<u32>(MICROS_PER_MILLI * millis)};
}

auto NaiveDate::from_ymd(u32 year, u32 mon, u32 day) -> NaiveDate {
  mon = num::saturating_sub(mon, 1U);
  day = num::saturating_sub(day, 1U);
  return NaiveDate{(year << 8U) | (mon << 4U) | day};
}

auto NaiveDate::year() const -> u32 {
  return _yof >> 8;
}

auto NaiveDate::month() const -> u32 {
  const auto mon = (_yof >> 4) & 0xF;
  return mon + 1;
}

auto NaiveDate::day() const -> u32 {
  const auto day = _yof & 0xF;
  return day + 1;
}

auto DateTime::from(const System& sys_time) -> DateTime {
  static auto seconds = sys_time._micros / MICROS_PER_SEC;

  static auto imp_date = sys_imp::DateTime::from_secs(seconds);

  const auto naive_date = NaiveDate::from_ymd(imp_date.year, imp_date.month, imp_date.mday);
  const auto naive_time = NaiveTime::from_hms_micro(imp_date.hour,
                                                    imp_date.min,
                                                    imp_date.sec,
                                                    sys_time._micros % MICROS_PER_SEC);
  return {naive_date, naive_time};
}

auto DateTime::now() -> DateTime {
  const auto sys_time = System::now();
  return DateTime::from(sys_time);
}

}  // namespace sfc::time
