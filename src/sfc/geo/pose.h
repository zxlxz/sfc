#pragma once

#include "point.h"
#include "trans.h"

namespace sfc::geo {

struct Pose {
  Point _pos{0, 0};
  Vector _vec{1, 0};

 public:
  Pose() = default;

  Pose(Point p, Vector v) : _pos{p}, _vec{v} {}

  auto position() const -> Point {
    return _pos;
  }

  auto direction() const -> Vector {
    return _vec;
  }

  // trans: local to global
  auto trans_l2g() const -> Affine {
    return Affine{}.rotate(_vec).translate(_pos.as_vec());
  }

  // trans: global to local
  auto trans_g2l() const -> Affine {
    return Affine{}.translate(-_pos.as_vec()).rotate(_vec.conj());
  }

  auto trans_to(const Pose& new_pose) const -> Affine {
    const auto t1 = Affine{}.rotate(_vec).translate(_pos.as_vec());
    const auto t2 = t1.translate(-new_pose._pos.as_vec()).rotate(new_pose._vec.conj());
    return t2;
  }

  void fmt(auto& f) const {
    auto imp = f.debug_struct();
    imp.field("pos", _pos);
    imp.field("vec", _vec);
  }
};

}  // namespace sfc::geo
