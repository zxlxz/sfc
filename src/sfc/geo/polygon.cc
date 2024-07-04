#include "polygon.h"

#include "trans.h"

namespace sfc::geo {

auto Polygon::vertex_order() const -> int {
  const auto cnt = _points.len();
  if (cnt < 3) {
    return 0;
  }

  auto pos_cnt = 0U;
  auto neg_cnt = 0U;

  _points.as_slice().windows(2).for_each([&, v = Vector{0, 0}](auto w) mutable {
    const auto cur_vec = w[1] - w[0];
    const auto cross_prod = v.cross_prod(cur_vec);
    if (cross_prod > 0) {
      pos_cnt += 1;
    }
    if (cross_prod < 0) {
      neg_cnt += 1;
    }
    v = cur_vec;
  });

  if (pos_cnt > 0 && neg_cnt == 0) {
    return +1;
  }

  if (neg_cnt > 0 && pos_cnt == 0) {
    return -1;
  }
  return 0;
}

auto Polygon::is_convex() const -> bool {
  const auto order = vertex_order();
  return order > 0;
}

auto Polygon::area() const -> double {
  if (_points.len() < 3) {
    return 0.0;
  }

  auto sum = 0.0;

  _points.as_slice().windows(2).for_each([&](auto w) {
    const auto a = w[0].as_vec();
    const auto b = w[1].as_vec();
    sum += a.cross_prod(b);
  });

  return 0.5 * math::fabs(sum);
}

auto Polygon::centroid() const -> Point {
  const auto cnt = _points.len();
  if (cnt < 3) {
    if (cnt == 0) {
      return Point{0, 0};
    }
    if (cnt == 1) {
      return _points[0];
    }
    if (cnt == 2) {
      return _points[0] + 0.5 * (_points[1] - _points[0]);
    }
  }

  auto sum_a = 0.0;
  auto sum_x = 0.0;
  auto sum_y = 0.0;
  _points.as_slice().windows(2).for_each([&](auto w) {
    const auto a = w[0].as_vec();
    const auto b = w[1].as_vec();
    sum_a += a.cross_prod(b);
    sum_x += (a.x() + b.x()) / 3;
    sum_y += (a.y() + b.y()) / 3;
  });

  const auto cx = sum_x / sum_a;
  const auto cy = sum_y / sum_a;
  return Point{cx, cy};
}

}  // namespace sfc::geo
