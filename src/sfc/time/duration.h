#pragma once

#include "sfc/core.h"

namespace sfc::time {

constexpr u64 MILLIS_PER_SEC = 1000U;
constexpr u64 MICROS_PER_SEC = 1000000U;
constexpr u64 NANOS_PER_SEC = 1000000000U;

constexpr u32 MICROS_PER_MILLI = 1000U;
constexpr u32 NANOS_PER_MICRO = 1000U;
constexpr u32 NANOS_PER_MILLI = 1000000U;

struct Duration {
  u64 _nanos = 0U;

 public:
  static auto from_secs(u64 secs) noexcept -> Duration {
    return {secs * NANOS_PER_SEC};
  }

  static auto from_millis(u64 millis) noexcept -> Duration {
    return {millis * NANOS_PER_MILLI};
  }

  static auto from_micros(u64 micros) noexcept -> Duration {
    return {micros * NANOS_PER_MICRO};
  }

  static auto from_nanos(u64 nanos) noexcept -> Duration {
    return {nanos};
  }

  static auto from_secs_f32(f32 secs) noexcept -> Duration {
    const auto nanos = static_cast<f64>(secs) * static_cast<f64>(NANOS_PER_SEC);
    return {static_cast<u64>(nanos)};
  }

  static auto from_secs_f64(f64 secs) noexcept -> Duration {
    const auto nanos = secs * static_cast<f64>(NANOS_PER_SEC);
    return {static_cast<u64>(nanos)};
  }

  auto as_secs() const noexcept -> u64 {
    return _nanos / NANOS_PER_SEC;
  }

  auto as_nanos() const noexcept -> u64 {
    return _nanos;
  }

  auto as_millis() const noexcept -> u64 {
    return _nanos / NANOS_PER_MILLI;
  }

  auto as_micros() const noexcept -> u64 {
    return _nanos / NANOS_PER_MICRO;
  }

  auto subsec_nanos() const noexcept -> u32 {
    return static_cast<u32>(_nanos % NANOS_PER_SEC);
  }

  auto subsec_millis() const noexcept -> u32 {
    return static_cast<u32>(_nanos % NANOS_PER_SEC / NANOS_PER_MILLI);
  }

  auto subsec_micros() const noexcept -> u32 {
    return static_cast<u32>(_nanos % NANOS_PER_SEC / NANOS_PER_MICRO);
  }

  auto as_secs_f32() const noexcept -> f32 {
    return static_cast<f32>(_nanos) / static_cast<f32>(NANOS_PER_SEC);
  }

  auto as_secs_f64() const noexcept -> f64 {
    return static_cast<f64>(_nanos) / static_cast<f64>(NANOS_PER_SEC);
  }

 public:
  // trait: ops::Eq
  auto operator==(const Duration& other) const noexcept -> bool {
    return _nanos == other._nanos;
  }

  // trait: fmt::Display
  void fmt(auto& f) const {
    const auto secs = this->as_secs_f64();
    f.write_fmt("{.3}s", secs);
  }
};

}  // namespace sfc::time
