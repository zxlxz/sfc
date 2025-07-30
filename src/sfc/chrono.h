#pragma once

#include "sfc/time.h"

namespace sfc::chrono {

struct DateTime {
  i16 _year;
  u8 _month;
  u8 _day;
  u8 _hour;
  u8 _minute;
  u8 _second;

 public:
  static auto from_utc(const time::System& utc_time) -> DateTime;

  static auto from_local(const time::System& local_time) -> DateTime;

  auto year() const -> int {
    return _year;
  }

  auto month() const -> int {
    return _month;
  }

  auto day() const -> int {
    return _day;
  }

  auto hour() const -> int {
    return _hour;
  }

  auto minute() const -> int {
    return _minute;
  }

  auto second() const -> int {
    return _second;
  }

  void fmt(auto& f) const {
    f.write_fmt("{04}-{02}-{02}T{02}:{02}:{02}.{03}", _year, _month, _day, _hour, _minute, _second);
  }
};

}  // namespace sfc::chrono
