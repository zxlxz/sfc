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
  // [[fill]align][sign]['#'][0][width][.][precision][type]
  static auto from_str(Str s) noexcept -> Option<Style>;
};

template <class... T>
struct Args {
  Str _fmts = {};
  Tuple<const T*...> _args = {};

 public:
  Args(Str fmts, const T&... args) noexcept : _fmts{fmts}, _args{&args...} {}

  void fmt(auto& f) const noexcept {
    auto pats = _fmts;
    _args.map([&](const auto* ptr) {
      const auto i0 = pats.find('{').unwrap_or(pats.len());
      f.write_str(pats[{0, i0}]);
      pats = pats[{i0 + 1, ops::$}];

      const auto i1 = pats.find('}').unwrap_or(pats.len());
      const auto ss = pats[{0, i1}];
      pats = pats[{i1 + 1, pats.len()}];

      f._style = Style::from_str(ss).unwrap_or({});
      f.write_val(*ptr);
    });
    f.write_str(pats);
  }
};

}  // namespace sfc::fmt
