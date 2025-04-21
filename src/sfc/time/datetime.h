#pragma once

#include "system.h"

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
  // YYYY YYYY YYYY FFDD
  u32 _yof = 0;

 public:
  static auto from_ymd(u32 year, u32 month, u32 date) -> NaiveDate;

  auto year() const -> u32;
  auto month() const -> u32;
  auto day() const -> u32;

  void fmt(auto& f) const {
    const auto year = this->year();
    const auto month = this->month();
    const auto day = this->day();
    f.write_fmt("{04}-{02}-{02}", year, month, day);
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

  void fmt(auto& f) const {
    f.write_fmt("{} {}", _date, _time);
  }
};

}  // namespace sfc::time
