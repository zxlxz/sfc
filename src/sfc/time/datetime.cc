#include "datetime.h"

#include "sfc/sys/time.inl"

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
  const auto secs = (hour * 60 + min) * 60 + sec;
  return {secs, static_cast<u32>(MICROS_PER_MILLI * millis)};
}

NaiveDate::NaiveDate(u32 year, u32 mon, u32 day) {
  mon  = num::saturating_sub(mon, 1U);
  day  = num::saturating_sub(day, 1U);
  _yof = (year << 8U) | (mon << 4U) | day;
}

auto NaiveDate::from_ymd(u32 year, u32 month, u32 date) -> NaiveDate {
  return {year, month, date};
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
  static auto last_secs = sys_time._micros / MICROS_PER_SEC;
  static auto date_time = sys_imp::DateTime::from_secs(last_secs);

  static auto curr_secs = sys_time._micros / MICROS_PER_SEC;
  if (last_secs != curr_secs) {
    last_secs = curr_secs;
    date_time = sys_imp::DateTime::from_secs(last_secs);
  }

  const auto date = NaiveDate::from_ymd(date_time.year, date_time.month, date_time.mday);
  const auto time = NaiveTime::from_hms_micro(date_time.hour,
                                              date_time.min,
                                              date_time.sec,
                                              sys_time._micros % MICROS_PER_SEC);
  return {date, time};
}

auto DateTime::now() -> DateTime {
  const auto sys_time = System::now();
  return DateTime::from(sys_time);
}

}  // namespace sfc::time
