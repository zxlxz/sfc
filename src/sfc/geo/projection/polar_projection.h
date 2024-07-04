#pragma once

#include "sfc/geo/angle.h"
#include "sfc/geo/point.h"

namespace sfc::geo {

class PolarProjection {
  Angle _gap = 0;
  Vec<f64> _dists;

 public:
  explicit PolarProjection(usize cnt, f64 init_val);
  ~PolarProjection();
  PolarProjection(PolarProjection&&) noexcept;

  void update(Point p);

  auto dists() const -> Slice<const f64>;

  auto to_points() const -> Vec<Point>;

  void optimize_concave(Angle angle);

 private:
  void optimize_concave_by_wlen(usize wlen);
  void smooth_head_tail(usize cnt);
};

}  // namespace sfc::geo
