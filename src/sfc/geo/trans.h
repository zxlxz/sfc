#pragma once

#include "angle.h"
#include "matrix.h"

namespace sfc::geo {

struct Affine {
  Mat2x2 _mat = Mat2x2::eye();
  Vector _vec = {0, 0};

 public:
  Affine() = default;

  Affine(const Mat2x2& mat, const Vector& vec) : _mat{mat}, _vec{vec} {}

  [[nodiscard]] auto zoom(f64 k) const -> Affine {
    const auto mat = Mat2x2{{k, 0}, {0, k}};
    const auto res = Affine{mat * _mat, mat * _vec};
    return res;
  }

  [[nodiscard]] auto scale(f64 kx, f64 ky) const -> Affine {
    const auto mat = Mat2x2{{kx, 0}, {0, ky}};
    const auto res = Affine{mat * _mat, mat * _vec};
    return res;
  }

  [[nodiscard]] auto reflect_over_x() const -> Affine {
    return this->scale(+1, -1);
  }

  [[nodiscard]] auto reflect_over_y() const -> Affine {
    return this->scale(-1, +1);
  }

  [[nodiscard]] auto translate(const Vector& vec) const -> Affine {
    const auto res = Affine{_mat, vec + _vec};
    return res;
  }

  [[nodiscard]] auto rotate(const Vector& vec) const -> Affine {
    const auto mat = Mat2x2{{vec._x, -vec._y}, {vec._y, vec._x}};
    const auto res = Affine{mat * _mat, mat * _vec};
    return res;
  }

  [[nodiscard]] auto rotate(const Angle& angle) const -> Affine {
    const auto vec = angle.to_vec();
    return this->rotate(vec);
  }

  [[nodiscard]] auto inv() const -> Affine {
    const auto mat = _mat.inv();
    const auto vec = mat * -_vec;
    return {mat, vec};
  }

  template <class T>
  auto operator()(const T& t) const -> T {
    return this->apply_imp(t);
  }

  template <class T>
  auto operator()(const T& t) const -> T
    requires(requires() { t.transform(*this); })
  {
    return t.transform(*this);
  }

 private:
  auto apply_imp(const Vector& vec) const -> Vector {
    const auto v = _mat * vec;
    return v;
  }

  auto apply_imp(const Point& p) const -> Point {
    const auto q = (_mat * p.as_vec()).as_point() + _vec;
    return q;
  }
};

}  // namespace sfc::geo
