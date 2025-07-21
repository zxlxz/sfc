#pragma once

#include "sfc/time/duration.h"

namespace sfc::time {

struct System {
  u64 _nanos;

 public:
  static auto now() -> System;

  auto elapsed() const -> Duration;

  inline auto secs() const -> u64 {
    return _nanos / NANOS_PER_SEC;
  }

  inline auto sub_nanos() const -> u32 {
    return static_cast<u32>(_nanos % NANOS_PER_SEC);
  }

  inline auto sub_micros() const -> u32 {
    return static_cast<u32>(_nanos % NANOS_PER_SEC / NANOS_PER_MICRO);
  }
  
  inline auto sub_millis() const -> u32 {
    return static_cast<u32>(_nanos % NANOS_PER_SEC / NANOS_PER_MILLI);
  }

  inline auto duration_since(const System& earlier) const -> Duration {
    return *this - earlier;
  }

  inline auto operator==(const System& other) const -> bool {
    return _nanos == other._nanos;
  }

  inline auto operator<(const System& other) const -> bool {
    return _nanos < other._nanos;
  }

  inline auto operator<=(const System& other) const -> bool {
    return _nanos <= other._nanos;
  }

  auto operator-(const System& earlier) const -> Duration;
  auto operator+(const Duration& dur) const -> System;

  auto operator-(const Duration& dur) const -> System;

  auto operator+=(const Duration& dur) -> System&;
  auto operator-=(const Duration& dur) -> System&;
};

}  // namespace sfc::time
