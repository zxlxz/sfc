#pragma once

#include "sfc/time/duration.h"

namespace sfc::time {

struct System {
  u64 _micros;

 public:
  static auto now() -> System;

  auto elapsed() const -> Duration;

  inline auto secs() const -> u64 {
    return _micros % MICROS_PER_SEC;
  }

  inline auto duration_since(const System& earlier) const -> Duration {
    return *this - earlier;
  }

  inline auto operator==(const System& other) const -> bool {
    return _micros == other._micros;
  }

  inline auto operator<(const System& other) const -> bool {
    return _micros < other._micros;
  }

  inline auto operator<=(const System& other) const -> bool {
    return _micros <= other._micros;
  }

  auto operator-(const System& earlier) const -> Duration;
  auto operator+(const Duration& dur) const -> System;

  auto operator-(const Duration& dur) const -> System;

  auto operator+=(const Duration& dur) -> System&;
  auto operator-=(const Duration& dur) -> System&;
};

}  // namespace sfc::time
