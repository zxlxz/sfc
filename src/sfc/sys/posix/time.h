#pragma once

#include "sfc/core.h"

namespace sfc::sys::posix {

struct Instant {
  u64 sec;
  u64 nsec;

 public:
  static auto now() noexcept -> Instant;
};

struct SystemTime {
  u64 micros;

 public:
  static auto now() noexcept -> SystemTime;
};

struct DateTime {
  u16 year = 0;
  u16 month = 0;
  u16 day = 0;
  u16 hour = 0;
  u16 minute = 0;
  u16 second = 0;

 public:
  static auto from_utc(const SystemTime& sys_time) -> DateTime;
  static auto from_local(const SystemTime& sys_time) -> DateTime;
};

}  // namespace sfc::sys::posix
