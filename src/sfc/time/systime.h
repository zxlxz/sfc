#pragma once

#include "sfc/time/duration.h"

namespace sfc::time {

struct SysTime {
  u64 _nanos = 0;

 public:
  static auto now() -> SysTime;

  auto elapsed() const -> Duration {
    const auto now = SysTime::now();
    return Duration{now._nanos - _nanos};
  }

  auto secs() const -> u64 {
    return _nanos / NANOS_PER_SEC;
  }

  auto subsec_nanos() const -> u32 {
    return static_cast<u32>(_nanos % NANOS_PER_SEC);
  }

  auto subsec_micros() const -> u32 {
    return static_cast<u32>(_nanos % NANOS_PER_SEC / NANOS_PER_MICRO);
  }

  auto subsec_millis() const -> u32 {
    return static_cast<u32>(_nanos % NANOS_PER_SEC / NANOS_PER_MILLI);
  }

  auto operator==(const SysTime& other) const -> bool {
    return _nanos == other._nanos;
  }

  auto operator<(const SysTime& other) const -> bool {
    return _nanos < other._nanos;
  }

  auto operator<=(const SysTime& other) const -> bool {
    return _nanos <= other._nanos;
  }

  auto operator+(const Duration& dur) const -> SysTime {
    return SysTime{_nanos + dur._nanos};
  }

  auto operator-(const Duration& dur) const -> SysTime {
    return SysTime{_nanos - dur._nanos};
  }

  void fmt(auto& f) const {
    f.write_fmt("{}ns", _nanos);
  }
};

}  // namespace sfc::time
