#include "polar_projection.h"

namespace sfc::geo {

PolarProjection::PolarProjection(usize cnt, f64 init_val) {
  if (cnt == 0) {
    return;
  }
  _gap = Angle{(2.0 * math::PI) / static_cast<f64>(cnt)};
  _dists.resize(cnt, init_val);
}

PolarProjection::~PolarProjection() {}

PolarProjection::PolarProjection(PolarProjection&&) noexcept = default;

auto PolarProjection::dists() const -> Slice<const f64> {
  return _dists.as_slice();
}

void PolarProjection::update(Point p) {
  const auto vec = p.as_vec();
  const auto ang = vec.to_angle();
  const auto idx = static_cast<u64>(ang.value() / _gap.value());

  if (idx >= _dists.len()) {
    return;
  }

  auto r2 = p.as_vec().length_sqr();
  auto& dst = _dists[idx];
  if (r2 < dst * dst) {
    dst = math::sqrt(r2);
  }
}

auto PolarProjection::to_points() const -> Vec<Point> {
  const auto cnt = _dists.len();

  auto res = Vec<Point>::with_capacity(cnt);

  auto a = 0.0;
  for (auto r : _dists.as_slice()) {
    const auto v = Angle{a}.to_vec();
    const auto p = (r * v).as_point();
    res.push(p);
    a += _gap.value();
  }

  return res;
}

void PolarProjection::optimize_concave(Angle angle) {
  if (_dists.len() < 3) {
    return;
  }

  const auto max_width = angle.value() / _gap.value();
  const auto max_wlen = static_cast<usize>(math::ceil(max_width));
  for (auto wlen = 3U; wlen < max_wlen; ++wlen) {
    this->optimize_concave_by_wlen(wlen);
  }

  this->smooth_head_tail(2);
}

void PolarProjection::optimize_concave_by_wlen(usize wlen) {
  _dists.windows_mut(wlen).for_each([&](auto w) {
    const auto a = w.get_unchecked(0);
    const auto b = w.get_unchecked(wlen - 1);
    const auto u = cmp::max(a, b);

    auto v = w[{1, wlen - 1}];
    for (auto t : v) {
      if (t < u) return;
    }
    v.fill(u);
  });
}

void PolarProjection::smooth_head_tail(usize cnt) {
  if (_dists.len() < cnt) {
    return;
  }

  auto head = _dists[{0, 2}];
  head.fill(head[cnt - 1]);

  auto tail = _dists[{_dists.len() - 2, _}];
  tail.fill(tail[0]);
}

}  // namespace sfc::geo
