#pragma once

#include "sfc/geo/aabox.h"
#include "sfc/geo/point.h"

namespace sfc::geo {

class LinearProjection {
  f64 _x_int;
  f64 _x_min;
  f64 _x_max;

  Vec<f64> _y_min;
  Vec<f64> _y_max;

 public:
  LinearProjection(AABox box, f64 interval);
  ~LinearProjection();
  LinearProjection(LinearProjection&&) noexcept;

  void update(Point p);

  auto to_points_min() const -> Vec<Point>;
  auto to_points_max() const -> Vec<Point>;
  auto to_points() const -> Vec<Point>;

  void amp_limited_smooth(f64 err);
  void concave_padding(f64 dist);

 private:
  void optimize_concave_by_wlen(usize wlen);
  void head_tail_smooth(usize wlen);
};

}  // namespace sfc::geo
