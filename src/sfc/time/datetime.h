#pragma once

#include "sfc/time/system.h"

namespace sfc::time {

struct NaiveTime {
  u32 _secs = 0;
  u32 _micros = 0;

 public:
  static auto from_hms(u32 hour, u32 min, u32 sec) -> NaiveTime;
  static auto from_hms_milli(u32 hour, u32 min, u32 sec, u32 milli) -> NaiveTime;
  static auto from_hms_micro(u32 hour, u32 min, u32 sec, u32 micro) -> NaiveTime;

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
    return _micros / static_cast<u32>(MICROS_PER_MILLI);
  }

  auto micros() const -> u32 {
    return static_cast<u32>(_micros);
  }

  void fmt(auto& f) const {
    const auto hour = this->hour();
    const auto minute = this->minute();
    const auto second = this->second();
    const auto millis = this->millis();
    f.write_fmt("{02}:{02}:{02}.{03}", hour, minute, second, millis);
  }
};

struct NaiveDate {
  u16 _year;
  u8  _month;
  u8  _day;

 public:
  static auto from_ymd(u32 year, u32 month, u32 date) -> NaiveDate {
    const auto y = static_cast<u16>(year);
    const auto m = static_cast<u8>(month);
    const auto d = static_cast<u8>(date);
    return {y, m, d};
  }

  auto year() const -> u32 {
    return _year;
  }

  auto month() const -> u32 {
    return _month;
  }

  auto day() const -> u32 {
    return _day;
  }

  void fmt(auto& f) const {
    f.write_fmt("{04}-{02}-{02}", _year, _month, _day);
  }
};

struct DateTime {
  NaiveDate _date{};
  NaiveTime _time{};

 public:
  DateTime() = default;

  DateTime(NaiveDate date, NaiveTime time) : _date{date}, _time{time} {}

  static auto from(const System& time) -> DateTime;

  static auto now() -> DateTime;

  auto date() const -> NaiveDate {
    return _date;
  }

  auto time() const -> NaiveTime {
    return _time;
  }

  auto year() const -> u32 {
    return _date._year;
  }

  auto month() const -> u32 {
    return _date._month;
  }

  auto day() const -> u32 {
    return _date._day;
  }

  auto hour() const -> u32 {
    return _time.hour();
  }

  auto minute() const -> u32 {
    return _time.minute();
  }

  auto second() const -> u32 {
    return _time.second();
  }

  void fmt(auto& f) const {
    f.write_fmt("{} {}", _date, _time);
  }
};

}  // namespace sfc::time
