#pragma once

#include "sfc/core.h"

namespace sfc::time {

struct SystemTime {
  u64 _micros = 0;

 public:
  static auto now() noexcept -> SystemTime;

  auto elapsed() const noexcept -> Duration;
  auto duration_since(SystemTime earlier) const noexcept -> Duration;

  auto as_secs() const noexcept -> u64;
  auto subsec_nanos() const noexcept -> u32;
  auto subsec_micros() const noexcept -> u32;
  auto subsec_millis() const noexcept -> u32;

 public:
  // trait:: ops::Eq
  auto operator==(const SystemTime& other) const noexcept -> bool;

  // trait: ops::Add
  auto operator+(const Duration& dur) const noexcept -> SystemTime;

  // trait: ops::Sub
  auto operator-(const Duration& dur) const noexcept -> SystemTime;

  // trait: fmt::Display
  void fmt(auto& f) const {
    f.write_fmt(fmt::Args{"{}us", _micros});
  }
};

}  // namespace sfc::time
