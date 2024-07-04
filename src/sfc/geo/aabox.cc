#include "aabox.h"

namespace sfc::geo {

auto AABox::with_size_center(Size size, Point center) -> AABox {
  const auto cx = center._x;
  const auto cy = center._y;
  const auto dx = size.length() / 2;
  const auto dy = size.width() / 2;
  return AABox{{cx - dx, cx + dx}, {cy - dy, cy + dy}};
}

auto AABox::has_overlap(const AABox& other) const -> bool {
  if (_x_max <= other._x_min || other._x_max <= _x_min) {
    return false;
  }
  if (_y_max <= other._y_min || other._y_max <= _y_min) {
    return false;
  }
  return true;
}

void AABox::merge_rect(const AABox& other) {
  _x_min = cmp::min(_x_min, other._x_min);
  _x_max = cmp::max(_x_max, other._x_max);
  _y_min = cmp::min(_y_min, other._y_min);
  _y_max = cmp::max(_y_max, other._y_max);
}

auto AABox::operator&(const AABox& other) const -> AABox {
  if (!this->has_overlap(other)) {
    return {};
  }

  const auto xmin = cmp::max(_x_min, other._x_min);
  const auto xmax = cmp::min(_x_max, other._x_max);
  const auto ymin = cmp::max(_y_min, other._y_min);
  const auto ymax = cmp::min(_y_max, other._y_max);

  return AABox{(f64[]){xmin, xmax}, (f64[]){ymin, ymax}};
}

auto AABox::operator|(const AABox& rect) const -> AABox {
  auto tmp = *this;
  tmp.merge_rect(rect);
  return tmp;
}

}  // namespace sfc::geo
