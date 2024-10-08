#pragma once

#include "style.h"

namespace sfc::fmt {

template <class T>
class IFmt {
  T _val;

 public:
  void fmt(auto& f) const {
    const auto info = reflect_struct(_val);

    auto imp = f.debug_struct();
    info.fields().map([&](const auto& item) { imp.field(item.name, item.value); });
  }
};

template <class... T>
struct Args {
  str::Str _pats = {};
  tuple::Tuple<const T*...> _args = {};

 public:
  Args(const auto& pats, const T&... args) noexcept : _pats{pats}, _args{&args...} {}

  void fmt(auto& f) const {
    auto pats = _pats;

    _args.map([&](auto& val) {
      const auto i0 = pats.find('{').unwrap_or(pats.len());
      f.write_str(pats[{0, i0}]);
      pats = pats[{i0 + 1, pats.len()}];

      const auto i1 = pats.find('}').unwrap_or(pats.len());
      const auto ss = pats[{0, i1}];
      f.set_style(Style::from_str(ss).unwrap_or({}));
      pats = pats[{i1 + 1, pats.len()}];

      f.write(*val);
    });
    f.write_str(pats);
  }
};

template <class... T>
Args(const auto&, const T&...) -> Args<T...>;

template <class W>
class Fmter {
  W& _out;
  Style _style = {};

 public:
  [[sfc_inline]] Fmter(W& out) : _out{out} {}

  [[sfc_inline]] auto style() const -> const Style& {
    return _style;
  }

  [[sfc_inline]] void set_style(const Style& s) {
    _style = s;
  }

  [[sfc_inline]] void write_str(str::Str s) {
    _out.write_str(s);
  }

  [[sfc_inline]] void write_chr(char c) {
    _out.write_str(str::Str{&c, 1});
  }

  [[sfc_inline]] void write_chs(char c, usize n) {
    char buf[8] = {c, c, c, c, c, c, c, c};
    for (; n != 0;) {
      const auto len = cmp::min(n, 8UL);
      _out.write_str({buf, len});
      n -= len;
    }
  }

  void pad(str::Str s) {
    const auto width = _style.width();

    // check first
    if (width <= s.len()) {
      this->write_str(s);
      return;
    }

    const auto fill = _style.fill();
    const auto npad = width - s.len();

    switch (_style.align()) {
      default:
      case '<':
        _out.write_str(s);
        this->write_chs(fill, npad);
        break;
      case '>':
        this->write_chs(fill, npad);
        _out.write_str(s);
        break;
      case '=':
      case '^':
        this->write_chs(fill, (npad + 0) / 2);
        _out.write_str(s);
        this->write_chs(fill, (npad + 1) / 2);
        break;
    }
  }

  void pad_num(bool is_neg, str::Str body) {
    const auto width = _style.width();
    const auto sign = _style.sign(is_neg);
    const auto fill = _style.fill();
    const auto align = fill == '0' ? '=' : _style.align();

    const auto npad = num::saturating_sub<usize>(width, sign.len() + body.len());

    switch (align) {
      default:
      case '<':
        this->write_str(sign);
        this->write_str(body);
        this->write_chs(fill, npad);
        break;
      case '>':
        this->write_chs(fill, npad);
        this->write_str(sign);
        this->write_str(body);
        break;
      case '=':
        this->write_str(sign);
        this->write_chs(fill, npad);
        this->write_str(body);
        break;
      case '^':
        this->write_chs(fill, (npad + 0) / 2);
        this->write_str(sign);
        this->write_str(body);
        this->write_chs(fill, (npad + 1) / 2);
    }
  }

  void write(const auto& val) {
    if constexpr (requires { val.fmt(*this); }) {
      val.fmt(*this);
    } else {
      trait::as<IFmt>(val).fmt(*this);
    }
  }

  void write_fmt(str::Str fmts, const auto&... args) {
    Args{fmts, args...}.fmt(*this);
  }

  auto debug_tuple();
  auto debug_list();
  auto debug_set();
  auto debug_map();
  auto debug_struct();
};

template <class W>
void write(W& out, str::Str fmts, const auto&... args) {
  Fmter{out}.write_fmt(fmts, args...);
}

}  // namespace sfc::fmt
