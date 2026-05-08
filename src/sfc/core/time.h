#pragma once

#include "sfc/core/mod.h"

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
  static auto from_secs(u64 secs) noexcept -> Duration;
  static auto from_millis(u64 millis) noexcept -> Duration;
  static auto from_micros(u64 micros) noexcept -> Duration;
  static auto from_nanos(u64 nanos) noexcept -> Duration;
  static auto from_secs_f32(f32 secs) noexcept -> Duration;
  static auto from_secs_f64(f64 secs) noexcept -> Duration;

  auto as_secs() const noexcept -> u64;
  auto as_secs_f32() const noexcept -> f32;
  auto as_secs_f64() const noexcept -> f64;

  auto as_nanos() const noexcept -> u64;
  auto as_millis() const noexcept -> u64;
  auto as_micros() const noexcept -> u64;

  auto subsec_nanos() const noexcept -> u32;
  auto subsec_millis() const noexcept -> u32;
  auto subsec_micros() const noexcept -> u32;

 public:
  // trait: ops::Eq
  auto operator==(const Duration& other) const noexcept -> bool;

  // trait: fmt::Display
  void fmt(auto& f) const {
    const auto secs = this->as_secs_f64();
    if (secs >= 1.0) {
      f.write_val(secs);
      f.write_str("s");
    } else if (secs >= 1e-3) {
      f.write_val(secs * 1e3);
      f.write_str("ms");
    } else if (secs >= 1e-6) {
      f.write_val(secs * 1e6);
      f.write_str("µs");
    } else {
      f.write_val(secs * 1e9);
      f.write_str("ns");
    }
  }
};

}  // namespace sfc::time
