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
  auto operator==(Instant other) const noexcept -> bool;
  auto operator<=>(Instant other) const noexcept -> int;

  auto operator+(Duration dur) const noexcept -> Instant;
  auto operator-(Duration dur) const noexcept -> Instant;
  auto operator-(Instant other) const noexcept -> Duration;

 public:
  void fmt(fmt::Formatter& f) const;
};

}  // namespace sfc::time
