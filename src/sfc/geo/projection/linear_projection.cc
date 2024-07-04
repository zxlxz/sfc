#include "linear_projection.h"

namespace sfc::geo {

namespace {

struct AmpLimitedSegmenter {
  Slice<f64> _vec;
  f64 _amp;

  struct Item {
    Slice<f64> vec;
    f64 min_val = 0.0;
    f64 max_val = 0.0;

    operator bool() const {
      return vec.len() > 0;
    }
  };

 public:
  auto next() -> Item {
    if (_vec.is_empty()) {
      return {};
    }

    auto min_val = _vec.get_unchecked(0);
    auto max_val = min_val;

    auto idx = 1U;
    for (; idx < _vec._len; ++idx) {
      auto t = _vec.get_unchecked(idx);
      const auto new_min = cmp::min(min_val, t);
      const auto new_max = cmp::max(max_val, t);
      if (new_max - new_min > _amp) {
        break;
      }
    }

    auto vv = _vec.split_at_mut(idx);
    _vec = vv.template get<1>();

    auto res = Item{vv.template get<0>(), min_val, max_val};
    return res;
  }
};

}  // namespace

LinearProjection::LinearProjection(AABox box, f64 interval)
    : _x_int{interval}, _x_min{box.x_min()}, _x_max{box.x_max()} {
  const auto len = box.length();
  const auto cnt = static_cast<u64>(math::ceil(len / interval));
  _x_max = _x_min + _x_int * static_cast<f64>(cnt - 1);

  _y_min.resize(cnt, box.y_min());
  _y_max.resize(cnt, box.y_max());
}

LinearProjection::~LinearProjection() {}

LinearProjection::LinearProjection(LinearProjection&&) noexcept = default;

void LinearProjection::update(Point p) {
  const auto ix = static_cast<usize>((p.x() - _x_min) / _x_int);
  const auto fy = p.y();

  if (ix >= _y_min.len()) {
    return;
  }
  auto& ymin = _y_min.get_unchecked_mut(ix);
  auto& ymax = _y_max.get_unchecked_mut(ix);
  if (fy < 0) ymin = cmp::max(fy, ymin);
  if (fy > 0) ymax = cmp::min(fy, ymax);
}

auto LinearProjection::to_points_min() const -> Vec<Point> {
  auto res = Vec<Point>::with_capacity(_y_min.len());

  _y_min.iter().for_each([&, x = _x_min](auto y) mutable {
    res.push({x, y});
    x += _x_int;
  });
  return res;
}

auto LinearProjection::to_points_max() const -> Vec<Point> {
  auto res = Vec<Point>::with_capacity(_y_max.len());

  _y_max.iter().for_each([&, x = _x_min](auto y) mutable {
    res.push({x, y});
    x += _x_int;
  });
  return res;
}

auto LinearProjection::to_points() const -> Vec<Point> {
  auto res = Vec<Point>::with_capacity(_y_min.len() + _y_max.len());

  _y_min.iter().for_each([&, x = _x_min](auto y) mutable {
    res.push({x, y});
    x += _x_int;
  });

  _y_max.iter().for_each([&, x = _x_max](auto y) mutable {
    res.push({x, y});
    x -= _x_int;
  });
  return res;
}

void LinearProjection::concave_padding(f64 dist) {
  if (_y_min.len() < 3 || _y_max.len() < 3) {
    return;
  }

  const auto max_width = dist / _x_int;
  const auto max_wlen = static_cast<usize>(math::ceil(max_width));
  for (auto wlen = 3U; wlen <= max_wlen; ++wlen) {
    this->optimize_concave_by_wlen(wlen);
  }

  this->head_tail_smooth(2);
}

void LinearProjection::optimize_concave_by_wlen(usize wlen) {
  _y_min.windows_mut(wlen).for_each([&](auto w) {
    const auto a = w.get_unchecked(0);
    const auto b = w.get_unchecked(wlen - 1);
    const auto u = cmp::min(a, b);

    auto v = w[{1, wlen - 1}];
    for (auto t : v) {
      if (t > u) return;
    }
    v.fill(u);
  });

  _y_max.windows_mut(wlen).for_each([&](auto w) {
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

void LinearProjection::amp_limited_smooth(f64 err) {
  auto smin = AmpLimitedSegmenter{_y_min.as_mut_slice(), err};
  for (; auto w = smin.next();) {
    auto& v = w.vec;
    v[{1, v.len() - 1}].fill(w.max_val);
  }

  auto smax = AmpLimitedSegmenter{_y_max.as_mut_slice(), err};
  for (; auto w = smax.next();) {
    auto& v = w.vec;
    v[{1, v.len() - 1}].fill(w.min_val);
  }
}

void LinearProjection::head_tail_smooth(usize cnt) {
  auto yy = Tuple<Slice<f64>, Slice<f64>>{_y_min.as_mut_slice(), _y_max.as_mut_slice()};

  yy.map([&](auto v) {
    if (v.len() < cnt) {
      return;
    }

    auto head = v[{0, cnt}];
    head.fill(head[cnt - 1]);

    auto tail = v[{v.len() - cnt, v.len()}];
    tail.fill(tail[0]);
  });
}

}  // namespace sfc::geo
