#pragma once

#include "sfc/core/str.h"
#include "sfc/core/trait.h"

namespace sfc::fmt {

struct alignas(8) Style {
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
  static auto from_str(str::Str s) -> option::Option<Style>;

  auto fill(char default_val = ' ') const -> char {
    return _fill ? _fill : default_val;
  }

  auto align() const -> char {
    return _align;
  }

  auto type() const -> char {
    return _type;
  }

  auto width() const -> u32 {
    return _width;
  }

  auto radix() const -> u32 {
    const auto t = _type | 32;
    return t == 'b' ? 2 : t == 'o' ? 8 : t == 'x' ? 16 : 10;
  }

  auto verbose() const -> bool {
    return _prefix == '#';
  }

  auto sign(bool is_neg) const -> str::Str {
    if (is_neg) {
      return "-";
    }
    switch (_sign) {
      case '+': return "+";
      case '-': return " ";
      default:  return "";
    }
  }

  auto precision(u32 default_val) const -> u32 {
    return _point ? _precision : default_val;
  }

  auto prefix() const -> str::Str {
    if (_prefix != '#') {
      return "";
    }
    switch (_type) {
      case 'X': return "0X";
      case 'x': return "0x";
      case 'O': return "0";
      case 'o': return "0";
      case 'B': return "0B";
      case 'b': return "0b";
      default:  return "";
    }
  }
};

struct Display {
  static void fmt(const auto& self, auto& f) {
    if constexpr (requires { self.fmt(f); }) {
      return self.fmt(f);
    } else {
      Display::fmt_imp(self, f);
    }
  }

 private:
  static auto fill_int(slice::Slice<char> buf, Style style, auto val) -> str::Str;
  static auto fill_flt(slice::Slice<char> buf, Style style, auto val) -> str::Str;

  static void fmt_imp(bool val, auto& f) {
    f.pad(val ? str::Str{"true"} : str::Str{"false"});
  }

  static void fmt_imp(char val, auto& f) {
    f.write_str(str::Str{&val, 1});
  }

  static void fmt_imp(const trait::uint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 16];
    const auto s = Display::fill_int(buf, f._style, val);
    f.pad_num(false, s);
  }

  static void fmt_imp(const trait::sint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 16];
    const auto s = Display::fill_int(buf, f._style, val >= 0 ? val : 0 - val);
    f.pad_num(val < 0, s);
  }

  static void fmt_imp(const trait::float_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 16];
    const auto s = Display::fill_flt(buf, f._style, val >= 0 ? val : 0 - val);
    f.pad_num(val < 0, s);
  }

  static void fmt_imp(const void* val, auto& f) {
    char buf[8 * sizeof(val)];
    const auto s = Display::fill_int(buf, f._style, static_cast<const void*>(val));
    f.pad_num(false, s);
  }

  static void fmt_imp(trait::enum_ auto val, auto& f) {
    if constexpr (requires { to_str(val); }) {
      const auto s = to_str(val);
      f.pad(s);
    } else {
      using U = __underlying_type(decltype(val));
      f.write_fmt("{}({})", str::type_name<decltype(val)>(), static_cast<U>(val));
    }
  }

  template <class T, usize N>
  static void fmt_imp(const T (&val)[N], auto& f) {
    if constexpr (trait::same_<const T, const char>) {
      f.pad(val);
    } else {
      auto imp = f.debug_list();
      for (auto& x : val) {
        imp.entry(x);
      }
    }
  }
};


template <class... T>
struct Args {
  str::Str _pats = {};
  tuple::Tuple<const T*...> _args = {};

 public:
  explicit Args(const auto& pats, const T&... args) noexcept : _pats{pats}, _args{&args...} {}

  void fmt(auto& f) const {
    auto pats = _pats;
    _args.map([&](auto ptr) {
      const auto i0 = pats.find('{').unwrap_or(pats.len());
      f.write_str(pats.slice(0, i0));
      pats = pats.slice(i0 + 1, pats.len());

      const auto i1 = pats.find('}').unwrap_or(pats.len());
      const auto ss = pats.slice(0, i1);
      pats = pats.slice(i1 + 1, pats.len());

      f._style = Style::from_str(ss).unwrap_or({});
      if constexpr (requires { ptr->fmt(f); }) {
        ptr->fmt(f);
      } else {
        Display::fmt(*ptr, f);
      }
    });
    f.write_str(pats);
  }
};

template <class W>
struct Fmter {
  W& _out;
  Style _style = {};
  int _depth = 0;

 public:
  void write_char(char c) {
    if constexpr (requires { _out.push(c); }) {
      _out.push(c);
    } else {
      _out.write_str({&c, 1});
    }
  }

  void write_str(str::Str s) {
    if (s.is_empty()) {
      return;
    }
    if constexpr (requires { _out.push_str(s); }) {
      _out.push_str(s);
    } else {
      _out.write_str(s);
    }
  }

  void fill(usize n) {
    const auto c = _style._fill ? _style._fill : _style._prefix ? '0' : ' ';
    const char v[] = {c, c, c, c, c, c, c, c, c, c, c, c, c, c, c, c};
    for (auto i = 0U; i < n; i += sizeof(v)) {
      this->write_str({v, n < sizeof(v) ? n : sizeof(v)});
    }
  }

  void pad(str::Str s) {
    if (_style._width <= s._len) {
      this->write_str(s);
      return;
    }

    const auto npad = usize{_style._width} - s._len;

    switch (_style._align) {
      default:
      case '>':
        this->fill(npad);
        this->write_str(s);
        break;
      case '<':
        this->write_str(s);
        this->fill(npad);
        break;
      case '=':
      case '^':
        this->fill((npad + 0) / 2);
        this->write_str(s);
        this->fill((npad + 1) / 2);
        break;
    }
  }

  void pad_num(bool is_neg, str::Str body) {
    const auto sign = _style.sign(is_neg);
    const auto prefix = _style.prefix();
    const auto align = (_style._prefix || _style._fill == '0') ? '=' : _style._align;
    const auto len = prefix.len() + sign.len() + body.len();
    const auto npad = _style._width > len ? _style._width - len : 0U;

    switch (align) {
      default:
      case '>':
        this->fill(npad);
        this->write_str(sign);
        this->write_str(prefix);
        this->write_str(body);
        break;
      case '<':
        this->write_str(sign);
        this->write_str(prefix);
        this->write_str(body);
        this->fill(npad);
        break;
      case '=':
        this->write_str(sign);
        this->write_str(prefix);
        this->fill(npad);
        this->write_str(body);
        break;
      case '^':
        this->fill((npad + 0) / 2);
        this->write_str(sign);
        this->write_str(prefix);
        this->write_str(body);
        this->fill((npad + 1) / 2);
    }
  }

  void write_fmt(str::Str fmts, const auto&... args) {
    if constexpr (sizeof...(args) == 0) {
      this->write_str(fmts);
    } else {
      Args{fmts, args...}.fmt(*this);
    }
  }

 public:
  class DebugTuple;
  auto debug_tuple() -> DebugTuple {
    return DebugTuple{*this};
  }

  class DebugList;
  auto debug_list() -> DebugList {
    return DebugList{*this};
  }

  class DebugSet;
  auto debug_set() -> DebugSet {
    return DebugSet{*this};
  }

  class DebugMap;
  auto debug_map() -> DebugMap {
    return DebugMap{*this};
  }

  class DebugStruct;
  auto debug_struct() -> DebugStruct {
    return DebugStruct{*this};
  }

 private:
  static constexpr auto INDENT_SIZE = 2U;
  static constexpr auto MAX_DEPTH = 20U;

  void debug_begin(str::Str s) noexcept {
    this->write_str(s);
    _depth += 1;
  }

  void debug_end(str::Str s, u32 cnt) noexcept {
    _depth -= 1;

    if (cnt && _style._prefix == '#') {
      const auto s = "\n                                                               ";
      const auto n = 1 + INDENT_SIZE * num::min<u32>(_depth, MAX_DEPTH);
      this->write_str({s, n});
    }
    this->write_str(s);
  }

  void debug_val(u32 idx, const auto& val, const auto&... keys) {
    if (_style._prefix != '#') {
      const auto s = ", ";
      const auto i = idx == 0 ? 0U : 1U;
      this->write_str({s + i, 2 - i});
    } else {
      const auto s = ",\n                                                               ";
      const auto n = 1U + INDENT_SIZE * num::min<u32>(_depth, MAX_DEPTH);
      const auto i = idx == 0 ? 0U : 1U;
      this->write_str({s + i, n - i});
    }

    if constexpr (sizeof...(keys) > 0) {
      (this->write_str(keys), ...);
    }

    const auto old_style = _style;
    _style = Style{._type = _style._type};
    if constexpr (requires { val.fmt(*this); }) {
      val.fmt(*this);
    } else {
      Display::fmt(val, *this);
    }
    _style = old_style;
  }
};

template <class W>
class Fmter<W>::DebugTuple {
  Fmter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugTuple(Fmter& fmt) noexcept : _fmt{fmt} {
    _fmt.debug_begin("(");
  }

  ~DebugTuple() noexcept {
    _fmt.debug_end(")", _cnt);
  }

  DebugTuple(const DebugTuple&) = delete;

  auto entry(const auto& value) -> DebugTuple& {
    _fmt.debug_val(_cnt++, value);
    return *this;
  }

  void entries(auto&& iter) {
    iter.for_each([&](auto&& val) { this->entry(val); });
  }
};

template <class W>
class Fmter<W>::DebugList {
  Fmter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugList(Fmter& fmt) noexcept : _fmt{fmt} {
    _fmt.debug_begin("[");
  }

  ~DebugList() noexcept {
    _fmt.debug_end("]", _cnt);
  }

  DebugList(const DebugList&) noexcept = delete;

  auto entry(const auto& value) -> DebugList& {
    _fmt.debug_val(_cnt++, value);
    return *this;
  }

  void entries(auto&& iter) {
    iter.for_each([&](auto&& val) { this->entry(val); });
  }
};

template <class W>
class Fmter<W>::DebugSet {
  Fmter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugSet(Fmter& fmt) noexcept : _fmt{fmt} {
    _fmt.debug_begin("{");
  }

  ~DebugSet() noexcept {
    _fmt.debug_end("}", _cnt);
  }

  DebugSet(const DebugSet&) noexcept = delete;

  auto entry(const auto& value) -> DebugSet& {
    _fmt.debug_val(_cnt++, value);
    return *this;
  }

  void entries(auto&& iter) {
    iter.for_each([&](auto&& val) { this->entry(val); });
  }
};

template <class W>
class Fmter<W>::DebugMap {
  Fmter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugMap(Fmter& fmt) noexcept : _fmt{fmt} {
    _fmt.debug_begin("{");
  }

  ~DebugMap() noexcept {
    _fmt.debug_end("}", _cnt);
  }

  DebugMap(const DebugMap&) noexcept = delete;

  auto entry(str::Str name, const auto& value) -> DebugMap& {
    _fmt.debug_val(_cnt++, value, "\"", name, "\": ");
    return *this;
  }

  void entries(auto&& iter) {
    iter.for_each([&](const auto& item) {
      const auto& [k, v] = item;
      this->entry(k, v);
    });
  }
};

template <class W>
class Fmter<W>::DebugStruct {
  Fmter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugStruct(Fmter& fmt) noexcept : _fmt{fmt} {
    _fmt.debug_begin("{");
  }

  ~DebugStruct() noexcept {
    _fmt.debug_end("}", _cnt);
  }

  DebugStruct(DebugStruct&&) noexcept = delete;

  auto field(str::Str name, const auto& value) -> DebugStruct& {
    _fmt.debug_val(_cnt++, value, name, ": ");
    return *this;
  }

  void fields(auto&& iter) {
    iter.for_each([&](const auto& item) {
      const auto& [k, v] = item;
      this->field(k, v);
    });
  }
};


void write(auto& out, str::Str fmts, const auto&... args) {
  Fmter{out}.write_fmt(fmts, args...);
}

}  // namespace sfc::fmt
