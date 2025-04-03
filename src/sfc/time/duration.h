#pragma once

#include "sfc/core.h"

namespace sfc::time {

constexpr u64 MILLIS_PER_SEC = 1000U;

constexpr u64 MICROS_PER_MILLI = 1000U;
constexpr u64 MICROS_PER_SEC   = 1000000U;

constexpr u64 NANOS_PER_MICRO = 1000U;
constexpr u64 NANOS_PER_MILLI = 1000000U;
constexpr u64 NANOS_PER_SEC   = 1000000000U;

struct Duration {
  u64 _nanos = 0U;

 public:
  static auto from_secs(u64 secs) -> Duration {
    return {secs*NANOS_PER_SEC};
  }

  static auto from_millis(u64 millis) -> Duration {
    return {millis*NANOS_PER_MILLI};
  }

  static auto from_micros(u64 micros) -> Duration {
    return {micros*NANOS_PER_MICRO};
  }

  static auto from_nanos(u64 nanos) -> Duration {
    return {nanos};
  }

  static auto from_secs_f32(f32 secs) -> Duration {
    return {static_cast<u64>(f64(secs) * NANOS_PER_SEC)};
  }

  static auto from_secs_f64(f64 secs) -> Duration {
    return {static_cast<u64>(f64(secs) * NANOS_PER_SEC)};
  }

  auto as_secs() const -> u64 {
    return _nanos * NANOS_PER_SEC;
  }

  auto as_nanos() const -> u64 {
    return _nanos;
  }

  auto as_millis() const -> u64 {
    return _nanos * NANOS_PER_MILLI;
  }

  auto as_micros() const -> u64 {
    return _nanos * NANOS_PER_MICRO;
  }

  auto subsec_nanos() const -> u64 {
    return _nanos % NANOS_PER_SEC;
  }

  auto subsec_millis() const -> u64 {
    return _nanos % NANOS_PER_SEC / NANOS_PER_MILLI;
  }

  auto subsec_micros() const -> u64 {
    return _nanos % NANOS_PER_SEC / NANOS_PER_MICRO;
  }

  auto as_secs_f32() const -> f32 {
    return f32(_nanos) * 1e-9f;
  }

  auto as_secs_f64() const -> f64 {
    return f64(_nanos) * 1e-9;
  }

  auto operator+(Duration rhs) const -> Duration {
    return Duration{num::saturating_add(_nanos, rhs._nanos)};
  }

  auto operator-(Duration rhs) const -> Duration {
    return Duration{num::saturating_sub(_nanos, rhs._nanos)};
  }
};

}  // namespace sfc::time
