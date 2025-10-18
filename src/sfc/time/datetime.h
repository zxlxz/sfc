#pragma once

#include "sfc/time/systime.h"

namespace sfc::time {

struct DateTime {
  u16 year = 0;
  u16 month = 0;
  u16 day = 0;
  u16 hour = 0;
  u16 minute = 0;
  u16 second = 0;
  u32 nanos = 0;

 public:
  static auto from_utc(SysTime utc_time) -> DateTime;
  static auto from_local(SysTime local_time) -> DateTime;

  void fmt(auto& f) const {
    const auto millis = nanos / NANOS_PER_MILLI;
    f.write_fmt("{04}-{02}-{02}T{02}:{02}:{02}.{03}", year, month, day, hour, minute, second, millis);
  }
};

}  // namespace sfc::time
