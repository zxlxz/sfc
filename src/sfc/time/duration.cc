#include "duration.h"

namespace sfc::time {

auto Duration::from_secs(u64 val) -> Duration {
  const auto secs = val;
  const auto nanos = 0U;
  return {secs, nanos};
}

auto Duration::from_millis(u64 val) -> Duration {
  const auto secs = val / MILLIS_PER_SEC;
  const auto nanos = (val % MILLIS_PER_SEC) * NANOS_PER_MILLI;
  return Duration{secs, nanos};
}

auto Duration::from_micros(u64 val) -> Duration {
  const auto secs = val / MICROS_PER_SEC;
  const auto nanos = (val % MICROS_PER_SEC) * NANOS_PER_MICRO;
  return Duration{secs, nanos};
}

auto Duration::from_nanos(u64 val) -> Duration {
  const auto secs = val / NANOS_PER_SEC;
  const auto nanos = val % NANOS_PER_SEC;
  return Duration{secs, nanos};
}

auto Duration::from_secs_f32(f32 val) -> Duration {
  if (val <= 0) {
    return {};
  }
  auto int_part = 0.0;
  auto flt_part = __builtin_modf(val, &int_part);

  const auto secs = static_cast<u64>(int_part);
  const auto nanos = static_cast<u64>(flt_part * static_cast<f64>(NANOS_PER_SEC));
  return {secs, nanos};
}

auto Duration::from_secs_f64(f64 val) -> Duration {
  if (val <= 0) {
    return {};
  }
  auto int_part = 0.0;
  auto flt_part = __builtin_modf(val, &int_part);

  const auto secs = static_cast<u64>(int_part);
  const auto nanos = static_cast<u64>(flt_part * static_cast<f64>(NANOS_PER_SEC));
  return {secs, nanos};
}

auto Duration::as_secs_f32() const -> f32 {
  const auto int_part = static_cast<f32>(_secs);
  const auto flt_part = static_cast<f32>(_nanos) / static_cast<f32>(NANOS_PER_SEC);
  return int_part + flt_part;
}

auto Duration::as_secs_f64() const -> f64 {
  const auto int_part = static_cast<f64>(_secs);
  const auto flt_part = static_cast<f64>(_nanos) / static_cast<f64>(NANOS_PER_SEC);
  return int_part + flt_part;
}


auto Duration::operator+(Duration rhs) const -> Duration {
  auto secs = num::saturating_add(_secs, rhs._secs);
  auto nanos = _nanos + rhs._nanos;
  if (nanos > NANOS_PER_SEC) {
    nanos -= NANOS_PER_SEC;
    secs = num::saturating_add<u64>(secs, 1U);
  }
  return {secs, nanos};
}

auto Duration::operator-(Duration rhs) const -> Duration {
  auto secs = num::saturating_sub(_secs, rhs._secs);
  auto nanos = _nanos - rhs._nanos;
  if (_nanos < rhs._nanos) {
    if (secs == 0) {
      nanos = 0;
    } else {
      secs -= 1;
      nanos += NANOS_PER_SEC;
    }
  }

  return {secs, nanos};
}

}  // namespace sfc::time
