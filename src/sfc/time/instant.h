#pragma once

#include "sfc/time/duration.h"

namespace sfc::time {

struct Instant {
  u64 _nanos = 0;

 public:
  static auto now() noexcept -> Instant;

  auto duration_since(Instant earlier) const noexcept -> Duration {
    return {_nanos - earlier._nanos};
  }

  auto elapsed() const noexcept -> Duration {
    const auto now = Instant::now();
    return {now._nanos - _nanos};
  }

 public:
  // trait: ops::Eq
  auto operator==(const Instant& other) const noexcept -> bool {
    return _nanos == other._nanos;
  }

  // trait: ops::Add
  auto operator+(const Duration& dur) const noexcept -> Instant {
    return Instant{_nanos + dur._nanos};
  }

  // trait: ops::Sub
  auto operator-(const Duration& dur) const noexcept -> Instant {
    return Instant{_nanos - dur._nanos};
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    f.write_fmt("{}ns", _nanos);
  }
};

}  // namespace sfc::time
