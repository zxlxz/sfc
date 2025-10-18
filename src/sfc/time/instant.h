#pragma once

#include "sfc/time/duration.h"

namespace sfc::time {

struct Instant {
  u64 _nanos = 0;

 public:
  static auto now() -> Instant;

  auto duration_since(const Instant& earlier) const -> Duration {
    const auto nanos = _nanos < earlier._nanos ? 0U : _nanos - earlier._nanos;
    return Duration{nanos};
  }

  auto elapsed() const -> Duration {
    const auto now = Instant::now();
    return Duration{now._nanos - _nanos};
  }

  auto operator==(const Instant& other) const -> bool {
    return _nanos == other._nanos;
  }

  auto operator<(const Instant& other) const -> bool {
    return _nanos < other._nanos;
  }

  auto operator<=(const Instant& other) const -> bool {
    return _nanos <= other._nanos;
  }

  auto operator+(const Duration& dur) const -> Instant {
    return Instant{_nanos + dur._nanos};
  }

  auto operator-(const Duration& dur) const -> Instant {
    return Instant{_nanos - dur._nanos};
  }

  void fmt(auto& f) const {
    f.write_fmt("{}ns", _nanos);
  }
};

}  // namespace sfc::time
