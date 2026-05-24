#include "sfc/core/time.h"
#include "sfc/core/num.h"

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
  const auto nanos_f64 = secs * f64{NANOS_PER_SEC};
  const auto nanos_u64 = num::cast_unsigned(num::trunc_to_int(nanos_f64));
  return Duration{nanos_u64};
}

auto Duration::from_secs_f64(f64 secs) noexcept -> Duration {
  const auto nanos_f64 = secs * f64{NANOS_PER_SEC};
  const auto nanos_u64 = num::cast_unsigned(num::trunc_to_int(nanos_f64));
  return Duration{nanos_u64};
}

auto Duration::as_secs() const noexcept -> u64 {
  return _nanos / NANOS_PER_SEC;
}

auto Duration::as_secs_f32() const noexcept -> f32 {
  const auto secs_f64 = f64(_nanos) / f64{NANOS_PER_SEC};
  return f32(secs_f64);
}

auto Duration::as_secs_f64() const noexcept -> f64 {
  const auto secs_f64 = f64(_nanos) / f64{NANOS_PER_SEC};
  return secs_f64;
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
  const auto nanos_u64 = _nanos % NANOS_PER_SEC;
  return u32(nanos_u64);
}

auto Duration::subsec_millis() const noexcept -> u32 {
  const auto millis_u64 = _nanos % NANOS_PER_SEC / NANOS_PER_MILLI;
  return u32(millis_u64);
}

auto Duration::subsec_micros() const noexcept -> u32 {
  const auto micros_u64 = _nanos % NANOS_PER_SEC / NANOS_PER_MICRO;
  return u32(micros_u64);
}

auto Duration::operator==(const Duration& other) const noexcept -> bool {
  return _nanos == other._nanos;
}

}  // namespace sfc::time
