#pragma once

#include "duration.h"

namespace sfc::time {

struct Instant {
  u64 _secs = 0;
  u64 _nanos = 0;

 public:
  Instant() = default;

  Instant(u64 secs, u64 nanos) : _secs{secs}, _nanos{nanos} {}

  static auto now() -> Instant;

  auto elpased() const -> Duration;

  auto duration_since(const Instant& earlier) const -> Duration;

  auto operator==(const Instant& other) const -> bool;
  auto operator<(const Instant& other) const -> bool;
  auto operator<=(const Instant& other) const -> bool;

  auto operator-(const Instant& earlier) const -> Duration;

  auto operator+(const Duration& dur) const -> Instant;
  auto operator-(const Duration& dur) const -> Instant;

  void operator+=(const Duration& dur);
  void operator-=(const Duration& dur);
};

}  // namespace sfc::time
