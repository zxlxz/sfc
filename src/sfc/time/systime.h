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
  auto operator==(SystemTime other) const noexcept -> bool;
  auto operator<=>(SystemTime other) const noexcept -> int;

  auto operator+(Duration dur) const noexcept -> SystemTime;
  auto operator-(Duration dur) const noexcept -> SystemTime;
  auto operator-(SystemTime other) const noexcept -> Duration;

 public:
  void fmt(fmt::Formatter& f) const;
};

}  // namespace sfc::time
