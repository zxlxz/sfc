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
  auto operator==(const Instant& other) const noexcept -> bool;

  // trait: ops::Lt
  auto operator<(const Instant& other) const noexcept -> bool;

  // trait: ops::Gt
  auto operator>(const Instant& other) const noexcept -> bool;

  // trait: ops::Le
  auto operator<=(const Instant& other) const noexcept -> bool;

  // trait: ops::Ge
  auto operator>=(const Instant& other) const noexcept -> bool;

  // trait: ops::Add
  auto operator+(const Duration& dur) const noexcept -> Instant;

  // trait: ops::Sub
  auto operator-(const Duration& dur) const noexcept -> Instant;

 public:
  // trait: fmt::Display
  void fmt(fmt::Formatter& f) const;
};

}  // namespace sfc::time
