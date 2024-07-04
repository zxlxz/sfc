#pragma once

#include "vector.h"

namespace sfc::geo {

using math::PI;

struct Angle {
  f64 _value = 0;

 public:
  [[sfc_inline]] Angle() = default;

  [[sfc_inline]] Angle(f64 rad) noexcept : _value{rad} {}

  // radian
  [[sfc_inline]] static auto from_rad(f64 radian) -> Angle {
    return Angle{radian};
  }

  // degree
  [[sfc_inline]] static auto from_deg(f64 degree) -> Angle {
    return Angle{degree * (PI / 180.0)};
  }

  // vec
  [[sfc_inline]] static auto from_vec(Vector vec) -> Angle {
    return Angle{math::atan2(vec._y, vec._x)};
  }

  [[sfc_inline]] auto value() const -> f64 {
    return _value;
  }

  [[sfc_inline]] auto as_rad() const -> f64 {
    return _value;
  }

  [[sfc_inline]] auto as_deg() const -> f64 {
    return _value * (180.0 / PI);
  }

  // [0, 2π)
  auto wrap_to_2pi() const -> f64 {
    const auto a = math::fmod(_value, 2 * PI);
    const auto b = a < 0 ? a + 2 * PI : a;
    return b;
  }

  //[-π, π)
  auto wrap_to_pi() const -> f64 {
    const auto a = math::fmod(_value, 2 * PI);
    const auto b = a <= -PI ? a + 2 * PI : a > PI ? a - 2 * PI : a;
    return b;
  }

  // [0, 360)
  auto wrap_to_360() const -> f64 {
    const auto d = this->as_deg();
    const auto a = math::fmod(d, 360.0);
    const auto b = a < 0 ? a + 360 : a;
    return b;
  }

  // [-180, 180)
  auto wrap_to_180() const -> f64 {
    const auto d = this->as_deg();
    const auto a = math::fmod(d, 360.0);
    const auto b = a <= -180 ? a + 360 : a > 180 ? a - 360 : a;
    return b;
  }

  auto cos() const -> f64 {
    return math::cos(_value);
  }

  auto sin() const -> f64 {
    return math::sin(_value);
  }

  auto to_vec() const -> Vector {
    return {this->cos(), this->sin()};
  }

  auto operator-() const -> Angle {
    return Angle{-_value};
  }

  auto operator+(Angle other) const -> Angle {
    return Angle{_value + other._value};
  }

  auto operator-(Angle other) const -> Angle {
    return Angle{_value - other._value};
  }

  friend auto operator*(f64 k, Angle angle) -> Angle {
    return Angle{k * angle._value};
  }

  auto operator/(f64 k) const -> Angle {
    return Angle{_value / k};
  }

  void fmt(auto& f) const {
    const auto deg = this->wrap_to_180();
    f.write(deg);
  }
};

[[sfc_inline]] inline auto Vector::to_angle() const -> Angle {
  return Angle::from_vec(*this);
}

}  // namespace sfc::geo
