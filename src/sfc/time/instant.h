#pragma once

#include "sfc/core.h"

namespace sfc::time {

struct Instant {
  u64 _nanos = 0;

 public:
  static auto now() noexcept -> Instant;

  auto elapsed() const noexcept -> Duration;
  auto duration_since(Instant earlier) const noexcept -> Duration;

 public:
  // trait: ops::Eq
  auto operator==(const Instant& other) const noexcept -> bool {
    return _nanos == other._nanos;
  }

  // trait: ops::Add
  auto operator+(const Duration& dur) const noexcept -> Instant {
    const auto ns = _nanos + dur.as_nanos();
    return Instant{ns};
  }

  // trait: ops::Sub
  auto operator-(const Duration& dur) const noexcept -> Instant {
    const auto ns = num::saturating_sub(_nanos, dur.as_nanos());
    return Instant{ns};
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    f.write_fmt("{}ns", _nanos);
  }
};

}  // namespace sfc::time
