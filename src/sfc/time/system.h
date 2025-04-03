#pragma once

#include "duration.h"

namespace sfc::time {

struct System {
  u64 _micros;

 public:
  static auto now() -> System;

  auto elpased() const -> Duration;

  auto duration_since(const System& earlier) const -> Duration;

  auto operator==(const System& other) const -> bool;
  auto operator<(const System& other) const -> bool;
  auto operator<=(const System& other) const -> bool;

  auto operator-(const System& earlier) const -> Duration;

  auto operator+(const Duration& dur) const -> System;
  auto operator-(const Duration& dur) const -> System;

  void operator+=(const Duration& dur);
  void operator-=(const Duration& dur);
};

}  // namespace sfc::time
