#pragma once

#include "sfc/core/str.h"

namespace sfc::fmt {

struct Style {
  char _fill   = 0;
  char _align  = 0;  // [<>^=]
  char _sign   = 0;  // [+- ]
  char _prefix = 0;  // [#]
  char _point  = 0;  // [.]
  char _type   = 0;  // [*]

  u8 _width     = 0;
  u8 _precision = 0;

 public:
  // [[fill]align][sign]['#'][0][width][.][precision][type]
  static auto from_str(str::Str s) -> Option<Style>;

  auto fill() const -> char {
    return _fill ? _fill : ' ';
  }

  auto align() const -> char {
    return _align;
  }

  auto sign(bool is_neg) const -> str::Str {
    if (is_neg) {
      return "-";
    }
    return _sign == '+' ? str::Str{"+"} : _sign == ' ' ? str::Str{" "} : str::Str{};
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

  auto radix() const -> u32 {
    const auto t = _type | 32;
    return t == 'b' ? 2 : t == 'o' ? 8 : t == 'x' ? 16 : 10;
  }
};

template <class T>
struct IFmt;

template <class W>
class DebugTuple {
  W&    _fmt;
  usize _cnt = 0;

 public:
  explicit DebugTuple(W& fmt) : _fmt{fmt} {
    _fmt.write_str("(");
  }

  ~DebugTuple() {
    _fmt.write_str(")");
  }

  DebugTuple(const DebugTuple&) = delete;

  void entry(const auto& val) {
    if (_cnt != 0) {
      _fmt.write_str(", ");
    }
    _fmt.write(val);
    _cnt += 1;
  }

  void entries(auto&& iter) {
    iter.for_each([&](auto&& val) { this->entry(val); });
  }
};

template <class W>
class DebugList {
  W&    _fmt;
  usize _cnt = 0;

 public:
  explicit DebugList(W& fmt) : _fmt{fmt} {
    _fmt.write_str("[");
  }

  ~DebugList() {
    _fmt.write_str("]");
  }

  DebugList(const DebugList&) noexcept = delete;

  void entry(const auto& val) {
    if (_cnt != 0) {
      _fmt.write_str(", ");
    }
    _fmt.write(val);
    _cnt += 1;
  }

  void entries(auto&& iter) {
    iter->for_each([&](auto&& val) { this->entry(val); });
  }
};

template <class W>
class DebugSet {
  W&    _fmt;
  usize _cnt = 0;

 public:
  explicit DebugSet(W& fmt) : _fmt{fmt} {
    _fmt.write_str("{");
  }

  ~DebugSet() {
    if (!_fmt) {
      return;
    }
    _fmt.write_str("}");
  }

  DebugSet(const DebugSet&) noexcept = delete;

  void entry(const auto& val) {
    if (_cnt != 0) {
      _fmt.write_str(", ");
    }
    _fmt.write(val);
    _cnt += 1;
  }

  void entries(auto&& iter) {
    iter.for_each([&](auto&& val) { this->entry(val); });
  }
};

template <class W>
class DebugMap {
  W&    _fmt;
  usize _cnt = 0;

 public:
  explicit DebugMap(W& fmt) : _fmt{fmt} {
    _fmt.write_str("{");
  }

  ~DebugMap() {
    _fmt.write_str("}");
  }

  DebugMap(const DebugMap&) noexcept = delete;

  void entry(const auto& name, const auto& value) {
    if (_cnt != 0) {
      _fmt.write_str(", ");
    }

    _fmt.write_str("\"");
    _fmt.write_str(name);
    _fmt.write_str("\": ");
    _fmt.write(value);

    _cnt += 1;
  }

  void entries(auto iter) {
    iter->for_each([&](auto&& item) {
      const auto& [k, v] = item;
      this->entry(k, v);
    });
  }
};

template <class W>
class DebugStruct {
  W&    _fmt;
  usize _cnt = 0;

 public:
  explicit DebugStruct(W& fmt) : _fmt{fmt} {
    _fmt.write_str("{");
  }

  ~DebugStruct() {
    _fmt.write_str("}");
  }

  DebugStruct(DebugStruct&&) noexcept = delete;

  void field(str::Str name, const auto& value) {
    if (_cnt != 0) {
      _fmt.write_str(", ");
    }

    _fmt.write_str(name);
    _fmt.write_str(": ");
    _fmt.write(value);

    _cnt += 1;
  }

  void fields(auto iter) {
    iter.for_each([&](auto&& item) {
      const auto& [k, v] = item;
      this->field(k, v);
    });
  }
};

template <class... T>
struct Args {
  str::Str                  _pats = {};
  tuple::Tuple<const T*...> _args = {};

 public:
  explicit Args(const auto& pats, const T&... args) noexcept : _pats{pats}, _args{&args...} {}

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

template <class W>
class Fmter {
  W&    _out;
  Style _style = {};

 public:
  explicit Fmter(W& out) : _out{out} {}

  auto style() const -> const Style& {
    return _style;
  }

  void set_style(const Style& s) {
    _style = s;
  }

  void write_str(const auto& s) {
    if constexpr (__is_convertible_to(decltype(s), str::Str)) {
      _out.write_str(s);
    } else {
      _out.write_str(s.as_str());
    }
  }

  void pad(str::Str s) {
    const auto width = _style.width();

    if (width <= s.len()) {
      this->write_str(s);
      return;
    }

    const auto fill = _style.fill();

    auto pad_fill = [&](usize n) {
      for (auto i = 0U; i < n; ++i) {
        _out.write_str({&fill, 1});
      }
    };

    const auto npad = width - s.len();

    switch (_style.align()) {
      default:
      case '<':
        _out.write_str(s);
        pad_fill(npad);
        break;
      case '>':
        pad_fill(npad);
        _out.write_str(s);
        break;
      case '=':
      case '^':
        pad_fill((npad + 0) / 2);
        _out.write_str(s);
        pad_fill((npad + 1) / 2);
        break;
    }
  }

  void pad_num(bool is_neg, str::Str body) {
    const auto fill = _style.fill();

    auto pad_fill = [&](usize n) {
      for (auto i = 0U; i < n; ++i) {
        _out.write_str({&fill, 1});
      }
    };

    const auto width = _style.width();
    const auto sign  = _style.sign(is_neg);
    const auto align = fill == '0' ? '=' : _style.align();
    const auto npad  = num::saturating_sub<usize>(width, sign.len() + body.len());

    switch (align) {
      default:
      case '<':
        this->write_str(sign);
        this->write_str(body);
        pad_fill(npad);
        break;
      case '>':
        pad_fill(npad);
        this->write_str(sign);
        this->write_str(body);
        break;
      case '=':
        _out.write_str(sign);
        pad_fill(npad);
        _out.write_str(body);
        break;
      case '^':
        pad_fill((npad + 0) / 2);
        _out.write_str(sign);
        _out.write_str(body);
        pad_fill((npad + 1) / 2);
    }
  }

  template <class T>
  void write(const T& val) {
    if constexpr (requires { val.fmt(*this); }) {
      val.fmt(*this);
    } else {
      IFmt<T>{val}.fmt(*this);
    }
  }

  void write_fmt(str::Str fmts, const auto&... args) {
    Args{fmts, args...}.fmt(*this);
  }

  auto debug_tuple() -> DebugTuple<Fmter> {
    return {*this};
  }

  auto debug_list() -> DebugList<Fmter> {
    return {*this};
  }

  auto debug_set() -> DebugSet<Fmter> {
    return {*this};
  }

  auto debug_map() -> DebugMap<Fmter> {
    return {*this};
  }

  auto debug_struct() -> DebugStruct<Fmter> {
    return {*this};
  }
};

void write(auto& out, str::Str fmts, const auto&... args) {
  Fmter{out}.write_fmt(fmts, args...);
}

}  // namespace sfc::fmt
