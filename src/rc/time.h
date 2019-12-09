#pragma once

#include "rc/core.h"

namespace rc::time {

static constexpr u64 MILLIS_PER_SEC = 1000;
static constexpr u64 MICROS_PER_SEC = 1000000;
static constexpr u64 NANOS_PER_SEC = 1000000000;

static constexpr u64 NANOS_PER_MICROS = 1000;
static constexpr u64 NANOS_PER_MILLIS = 1000000;

struct Duration {
  u64 _secs;
  u32 _nanos;

  pub static auto from_secs(u64 secs) noexcept -> Duration;
  pub static auto from_millis(u64 millis) noexcept -> Duration;
  pub static auto from_micros(u64 micros) noexcept -> Duration;
  pub static auto from_nanos(u64 nanos) noexcept -> Duration;

  pub auto total_nanos() const noexcept -> u64;
  pub auto as_secs_f64() const noexcept -> f64;
  pub auto as_millis_f64() const noexcept -> f64;
};

struct Instant {
  u64 _secs;
  u32 _nanos;

  pub static auto now() noexcept -> Instant;
  pub static auto from_nanos(u64 nanos) noexcept -> Instant;

  pub auto total_nanos() const noexcept -> u64;
  pub auto as_secs_f64() const noexcept -> f64;
  pub auto duration_since(const Instant& earlier) const -> Duration;
  pub auto elpased() const -> Duration;

  pub auto operator+(const Duration& dur) const noexcept -> Instant;
  pub auto operator-(const Duration& dur) const noexcept -> Instant;
};

struct SystemTime final {
  u64 _secs;
  u32 _nanos;

  pub static auto now() noexcept -> SystemTime;
  pub static auto from_nanos(u64 nanos) noexcept -> SystemTime;

  pub auto total_nanos() const noexcept -> u64;
  pub auto duration_since(const SystemTime& earlier) const noexcept -> Duration;
  pub auto elpased() const noexcept -> Duration;

  pub auto operator+(const Duration& dur) const noexcept -> SystemTime;
  pub auto operator-(const Duration& dur) const noexcept -> SystemTime;
};

}  // namespace rc::time
