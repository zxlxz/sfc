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

  auto duration_since(Instant earlier) const -> Duration;

  auto operator==(const Instant& other) const -> bool;
  auto operator<(const Instant& other) const -> bool;
  auto operator<=(const Instant& other) const -> bool;

  auto operator-(Instant earlier) const -> Duration;

  auto operator+(Duration dur) const -> Instant;
  auto operator-(Duration dur) const -> Instant;

  void operator+=(Duration dur);
  void operator-=(Duration dur);
};

}  // namespace sfc::time
