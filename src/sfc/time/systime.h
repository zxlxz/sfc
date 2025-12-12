#pragma once

#include "sfc/time/duration.h"

namespace sfc::time {

struct SysTime {
  u64 _nanos = 0;

 public:
  static auto now() noexcept -> SysTime;

  auto elapsed() const noexcept -> Duration {
    const auto now = SysTime::now();
    return Duration{now._nanos - _nanos};
  }

  auto secs() const noexcept -> u64 {
    return _nanos / NANOS_PER_SEC;
  }

  auto subsec_nanos() const noexcept -> u32 {
    return static_cast<u32>(_nanos % NANOS_PER_SEC);
  }

  auto subsec_micros() const noexcept -> u32 {
    return static_cast<u32>(_nanos % NANOS_PER_SEC / NANOS_PER_MICRO);
  }

  auto subsec_millis() const noexcept -> u32 {
    return static_cast<u32>(_nanos % NANOS_PER_SEC / NANOS_PER_MILLI);
  }

 public:
 // trait:: ops::Eq
   auto operator==(const SysTime& other) const noexcept -> bool {
    return _nanos == other._nanos;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    f.write_fmt("{}ns", _nanos);
  }
};

}  // namespace sfc::time
