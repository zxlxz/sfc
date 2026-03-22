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
  u32 micros = 0;

 public:
  static auto from_utc(SystemTime utc_time) noexcept -> DateTime;
  static auto from_local(SystemTime local_time) noexcept -> DateTime;

  void fmt(auto& f) const {
    f.write_fmt("{04}-{02}-{02} {02}:{02}:{02}.{06}", year, month, day, hour, minute, second, micros);
  }
};

}  // namespace sfc::time
