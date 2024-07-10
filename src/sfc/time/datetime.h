#pragma once

#include "instant.h"

namespace sfc::time {

struct NaiveTime {
  u32 _secs = 0;
  u32 _nanos = 0;

 public:
  NaiveTime() = default;
  NaiveTime(u32 secs, u32 nanos) : _secs{secs}, _nanos{nanos} {}

  static auto from_hms(u32 hour, u32 min, u32 sec) -> NaiveTime;
  static auto from_hms_nano(u32 hour, u32 min, u32 sec, u32 nanos) -> NaiveTime;

  auto hour() const -> u32 {
    return _secs / 60 / 60;
  }

  auto minute() const -> u32 {
    return _secs / 60 % 60;
  }

  auto second() const -> u32 {
    return _secs % 60;
  }

  auto millis() const -> u32 {
    return _nanos / NANOS_PER_MILLI;
  }

  auto micros() const -> u32 {
    return _nanos / NANOS_PER_MICRO;
  }

  auto nanos() const -> u32 {
    return _nanos;
  }

  void fmt(auto& f) const {
    const auto millis = this->millis();
    const auto hour = this->hour();
    const auto min = this->minute();
    const auto sec = this->second();
    f.write_fmt("{02}:{02}:{02}.{06}", hour, min, sec, millis);
  }
};

struct NaiveDate {
  u32 _yof = 0;  // YYYY YYYY YYYY FFDD

 public:
  NaiveDate() = default;
  NaiveDate(u32 year, u32 month, u32 day);

  static auto from_ymd(u32 year, u32 month, u32 date) -> NaiveDate;

  auto year() const -> u32;
  auto month() const -> u32;
  auto day() const -> u32;

  auto operator==(const NaiveDate& other) const -> bool {
    return _yof == other._yof;
  }

  void fmt(auto& f) const {
    const auto year = this->year();
    const auto mon = this->month();
    const auto day = this->day();
    f.write_fmt("{04}:{02}:{02}", year, mon, day);
  }
};

struct DateTime {
  NaiveDate _date{};
  NaiveTime _time{};

 public:
  DateTime() = default;

  DateTime(NaiveDate date, NaiveTime time) : _date{date}, _time{time} {}

  static auto now_local() -> DateTime;

  auto date() const -> NaiveDate {
    return _date;
  }

  auto time() const -> NaiveTime {
    return _time;
  }

  void fmt(auto& f) const {
    f.write_fmt("{}T{}", _date, _time);
  }
};

}  // namespace sfc::time
