#include "datetime.h"

#include "sfc/sys/time.inl"

namespace sfc::time {

namespace sys_imp = sys::time;

auto NaiveTime::from_hms(u32 hour, u32 min, u32 sec) -> NaiveTime {
  const auto secs = (hour * 24 + min * 60) * 60 + sec;
  return {secs, 0};
}

auto NaiveTime::from_hms_nano(u32 hour, u32 min, u32 sec, u32 nanos) -> NaiveTime {
  const auto secs = (hour * 24 + min * 60) * 60 + sec;
  return {secs, nanos};
}

auto NaiveTime::hour() const -> u32 {
  const auto mins = _secs / 60;
  return mins / 60;
}

auto NaiveTime::minute() const -> u32 {
  const auto mins = _secs / 60;
  return mins % 60;
}

auto NaiveTime::second() const -> u32 {
  return _secs % 60;
}

auto NaiveTime::nanosecond() const -> u32 {
  return _nanos;
}

NaiveDate::NaiveDate(u32 year, u32 mon, u32 day) {
  mon = num::saturating_sub(mon, 1U);
  day = num::saturating_sub(day, 1U);
  _yof = (year << 8U) | (mon << 4U) | day;
}

auto NaiveDate::from_ymd(u32 year, u32 month, u32 date) -> NaiveDate {
  return {year, month, date};
}

auto NaiveDate::year() const -> u32 {
  return _yof >> 8;
}

auto NaiveDate::month() const -> u32 {
  auto mon = (_yof >> 4) & 0xFF;
  return mon + 1;
}

auto NaiveDate::day() const -> u32 {
  auto day = (_yof >> 4) & 0xFF;
  return day + 1;
}

auto DateTime::date() const -> NaiveDate {
  return _date;
}

auto DateTime::time() const -> NaiveTime {
  return _time;
}

auto DateTime::now_local() -> DateTime {
  const auto imp = sys_imp::DateTime::now();
  const auto date = NaiveDate::from_ymd(imp.year, imp.month, imp.mday);
  const auto time = NaiveTime::from_hms_nano(imp.hour, imp.min, imp.sec, imp.usec);
  return {date, time};
}

}  // namespace sfc::time
