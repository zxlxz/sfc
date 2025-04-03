#pragma once

#include "sfc/core/num.h"
#include "sfc/core/reflect.h"
#include "sfc/core/str.h"

namespace sfc::fmt {

struct Style {
  char _fill = 0;
  char _align = 0;   // [<>^=]
  char _sign = 0;    // [+- ]
  char _prefix = 0;  // [#]
  char _point = 0;   // [.]
  char _type = 0;    // [*]

  u8 _width = 0;
  u8 _precision = 0;

 public:
  Style() = default;

  // [[fill]align][sign]['#'][0][width][.][precision][type]
  static auto from_str(str::Str s) -> Option<Style>;

  auto fill() const -> char {
    return _fill ?: ' ';
  }

  auto align() const -> char {
    return _align;
  }

  auto sign(bool is_neg) const -> str::Str {
    if (is_neg) return "-";
    if (_sign == '+') return "+";
    if (_sign == '-') return " ";
    return "";
  }

  auto type() const -> char {
    return _type;
  }

  auto width() const -> u32 {
    return _width;
  }

  auto precision(u32 default_val) const -> u32 {
    return _point ? _precision : default_val;
  }
};

}  // namespace sfc::fmt
