#pragma once

#include "sfc/core.h"

namespace sfc::time {

struct SystemTime {
  u64 _micros = 0;

 public:
  static auto now() noexcept -> SystemTime;

  auto elapsed() const noexcept -> Duration {
    const auto now = SystemTime::now();
    return Duration::from_micros(now._micros - _micros);
  }

  auto as_secs() const noexcept -> u64 {
    return _micros / MICROS_PER_SEC;
  }

  auto subsec_nanos() const noexcept -> u32 {
    return static_cast<u32>(_micros % NANOS_PER_SEC * NANOS_PER_MICRO);
  }

  auto subsec_micros() const noexcept -> u32 {
    return static_cast<u32>(_micros % MICROS_PER_SEC);
  }

  auto subsec_millis() const noexcept -> u32 {
    return static_cast<u32>(_micros % MICROS_PER_SEC / MICROS_PER_MILLI);
  }

 public:
  // trait:: ops::Eq
  auto operator==(const SystemTime& other) const noexcept -> bool {
    return _micros == other._micros;
  }
};

}  // namespace sfc::time
