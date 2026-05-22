#include "sfc/core/time.h"

namespace sfc::time {

auto Duration::from_secs(u64 secs) noexcept -> Duration {
  return {secs * NANOS_PER_SEC};
}

auto Duration::from_millis(u64 millis) noexcept -> Duration {
  return {millis * NANOS_PER_MILLI};
}

auto Duration::from_micros(u64 micros) noexcept -> Duration {
  return {micros * NANOS_PER_MICRO};
}

auto Duration::from_nanos(u64 nanos) noexcept -> Duration {
  return Duration{nanos};
}

auto Duration::from_secs_f32(f32 secs) noexcept -> Duration {
  const auto nanos = static_cast<f64>(secs) * static_cast<f64>(NANOS_PER_SEC);
  return Duration{static_cast<u64>(nanos)};
}

auto Duration::from_secs_f64(f64 secs) noexcept -> Duration {
  const auto nanos = secs * static_cast<f64>(NANOS_PER_SEC);
  return Duration{static_cast<u64>(nanos)};
}

auto Duration::as_secs() const noexcept -> u64 {
  return _nanos / NANOS_PER_SEC;
}

auto Duration::as_secs_f32() const noexcept -> f32 {
  const auto ret = static_cast<f64>(_nanos) / static_cast<f64>(NANOS_PER_SEC);
  return static_cast<f32>(ret);
}

auto Duration::as_secs_f64() const noexcept -> f64 {
  return static_cast<f64>(_nanos) / static_cast<f64>(NANOS_PER_SEC);
}

auto Duration::as_nanos() const noexcept -> u64 {
  return _nanos;
}

auto Duration::as_millis() const noexcept -> u64 {
  return _nanos / NANOS_PER_MILLI;
}

auto Duration::as_micros() const noexcept -> u64 {
  return _nanos / NANOS_PER_MICRO;
}

auto Duration::subsec_nanos() const noexcept -> u32 {
  const auto val = _nanos % NANOS_PER_SEC;
  return static_cast<u32>(val);
}

auto Duration::subsec_millis() const noexcept -> u32 {
  const auto val = _nanos % NANOS_PER_SEC / NANOS_PER_MILLI;
  return static_cast<u32>(val);
}

auto Duration::subsec_micros() const noexcept -> u32 {
  const auto val = _nanos % NANOS_PER_SEC / NANOS_PER_MICRO;
  return static_cast<u32>(val);
}

auto Duration::operator==(const Duration& other) const noexcept -> bool {
  return _nanos == other._nanos;
}

}  // namespace sfc::time
