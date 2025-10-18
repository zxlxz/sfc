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

struct Debug {
  static auto fmt_int(slice::Slice<char> buf, auto val, Style style = {}) -> str::Str;
  static auto fmt_flt(slice::Slice<char> buf, auto val, Style style = {}) -> str::Str;
  static auto fmt_ptr(slice::Slice<char> buf, const void* val, Style style = {}) -> str::Str;

 public:
  static void fmt(bool val, auto& f) {
    f.pad(val ? str::Str{"true"} : str::Str{"false"});
  }

  static void fmt(char val, auto& f) {
    f.write_char(val);
  }

  static void fmt(const trait::uint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 16];
    const auto s = fmt_int(buf, val, f._style);
    f.pad_num(false, s);
  }

  static void fmt(const trait::sint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 16];
    const auto s = fmt_int(buf, val >= 0 ? val : 0 - val, f._style);
    f.pad_num(val < 0, s);
  }

  static void fmt(const trait::flt_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 16];
    const auto s = fmt_flt(buf, val >= 0 ? val : 0 - val, f._style);
    f.pad_num(val < 0, s);
  }

  static void fmt(const void* val, auto& f) {
    char buf[8 * sizeof(val)];
    const auto s = fmt_ptr(buf, val, f._style);
    f.pad_num(false, s);
  }

  static void fmt(trait::enum_ auto val, auto& f) {
    if constexpr (requires { to_str(val); }) {
      const auto s = to_str(val);
      f.pad(s);
    } else {
      using U = __underlying_type(decltype(val));
      f.write_fmt("{}({})", str::type_name<decltype(val)>(), static_cast<U>(val));
    }
  }

  template <class T, usize N>
  static void fmt(const T (&val)[N], auto& f) {
    slice::Slice{val}.fmt(f);
  }

  template <usize N>
  static void fmt(const char (&val)[N], auto& f) {
    str::Str{val}.fmt(f);
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
    _args.map([&](const auto* ptr) {
      const auto i0 = pats.find('{').unwrap_or(pats.len());
      f.write_str(pats[{0, i0}]);
      pats = pats[{i0 + 1, ops::$}];

      const auto i1 = pats.find('}').unwrap_or(pats.len());
      const auto ss = pats[{0, i1}];
      pats = pats[{i1 + 1, pats.len()}];

      f._style = Style::from_str(ss).unwrap_or({});
      if constexpr (requires { ptr->fmt(f); }) {
        ptr->fmt(f);
      } else {
        Debug::fmt(*ptr, f);
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
  void write_char(char c, usize n = 1) {
    const char v[] = {c, c, c, c, c, c, c, c};
    for (auto i = 0U; i < n; i += sizeof(v)) {
      this->write_str({v, n < sizeof(v) ? n : sizeof(v)});
    }
  }

  void write_str(str::Str s) {
    if (s.is_empty()) {
      return;
    }
    _out.write_str(s);
  }

  void fill(usize n) {
    const auto c = _style._fill ? _style._fill : _style._prefix ? '0' : ' ';
    const char v[] = {c, c, c, c, c, c, c, c};
    for (auto i = 0U; i < n; i += sizeof(v)) {
      const auto w = n - i < sizeof(v) ? n - i : sizeof(v);
      this->write_str({v, w});
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
      case '<':
        this->write_str(s);
        this->fill(npad);
        break;
      case '>':
        this->fill(npad);
        this->write_str(s);
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

  void write_val(const auto& val) {
    if constexpr (requires { val.fmt(*this); }) {
      val.fmt(*this);
    } else {
      Debug::fmt(val, *this);
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
  static constexpr auto INDENT_SIZE = 4U;
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

  void debug_next(u32 idx, const auto& val, const auto&... keys) {
    if (_style._prefix != '#') {
      if (idx != 0) {
        this->write_str(", ");
      }
    } else {
      const auto s = ",\n                                                               ";
      const auto n = 2U + INDENT_SIZE * num::min<u32>(_depth, MAX_DEPTH);
      const auto i = idx == 0 ? 1U : 0U;
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
      Debug::fmt(val, *this);
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
    _fmt.debug_next(_cnt++, value);
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
    _fmt.debug_next(_cnt++, value);
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
    _fmt.debug_next(_cnt++, value);
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
    _fmt.debug_next(_cnt++, value, "\"", name, "\": ");
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
    _fmt.debug_next(_cnt++, value, name, ": ");
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
