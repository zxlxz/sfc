#pragma once

#include "sfc/core.h"

namespace sfc::time {

constexpr u64 MILLIS_PER_SEC = 1000U;

constexpr u64 MICROS_PER_MILLI = 1000U;
constexpr u64 MICROS_PER_SEC = 1000000U;

constexpr u64 NANOS_PER_MICRO = 1000U;
constexpr u64 NANOS_PER_MILLI = 1000000U;
constexpr u64 NANOS_PER_SEC = 1000000000U;

struct Duration {
  u64 _secs = 0U;
  u64 _nanos = 0U;

 public:
  Duration() noexcept = default;
  Duration(u64 secs, u64 nanos) noexcept : _secs{secs}, _nanos{nanos} {}

  static auto from_secs(u64 secs) -> Duration;
  static auto from_millis(u64 millis) -> Duration;
  static auto from_micros(u64 micros) -> Duration;
  static auto from_nanos(u64 nanos) -> Duration;

  static auto from_secs_f32(f32 secs) -> Duration;
  static auto from_secs_f64(f64 secs) -> Duration;

  auto as_secs() const -> u64 {
    return _secs;
  }

  auto as_nanos() const -> u64 {
    return _secs * NANOS_PER_SEC + _nanos;
  }

  auto as_millis() const -> u64 {
    return _secs * MILLIS_PER_SEC + _nanos / NANOS_PER_MILLI;
  }

  auto as_micros() const -> u64 {
    return _secs * MICROS_PER_SEC + _nanos / NANOS_PER_MICRO;
  }

  auto subsec_nanos() const -> u64 {
    return _nanos;
  }

  auto subsec_millis() const -> u64 {
    return _nanos / NANOS_PER_MILLI;
  }

  auto subsec_micros() const -> u64 {
    return _nanos / NANOS_PER_MICRO;
  }

  auto as_secs_f32() const -> f32;
  auto as_secs_f64() const -> f64;

  auto operator+(Duration rhs) const -> Duration;
  auto operator-(Duration rhs) const -> Duration;
};

}  // namespace sfc::time
