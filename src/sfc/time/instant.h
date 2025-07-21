#pragma once

#include "sfc/time/duration.h"

namespace sfc::time {

struct Instant {
  u64 _nanos = 0;

 public:
  static auto now() -> Instant;

  auto elapsed() const -> Duration;

  auto operator==(const Instant& other) const -> bool {
    return _nanos == other._nanos;
  }

  auto operator<(const Instant& other) const -> bool {
    return _nanos < other._nanos;
  }

  auto operator<=(const Instant& other) const -> bool {
    return _nanos <= other._nanos;
  }

  auto operator-(const Instant& earlier) const -> Duration {
    return Duration{_nanos < earlier._nanos ? 0U : _nanos - earlier._nanos};
  }

  auto operator+(const Duration& dur) const -> Instant {
    return Instant{_nanos + dur._nanos};
  }

  auto operator-(const Duration& dur) const -> Instant {
    return Instant{_nanos - dur._nanos};
  }

  auto operator+=(const Duration& dur) -> Instant& {
    _nanos += dur._nanos;
    return *this;
  }

  auto operator-=(const Duration& dur) -> Instant& {
    _nanos -= dur._nanos;
    return *this;
  }
};

}  // namespace sfc::time
