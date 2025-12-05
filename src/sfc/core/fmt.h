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
  static auto from_str(Str s) noexcept -> Option<Style>;

  auto fill(char default_val = ' ') const noexcept -> char {
    return _fill ? _fill : default_val;
  }

  auto align() const noexcept -> char {
    return _align;
  }

  auto width() const noexcept -> u32 {
    return _width;
  }

  auto type(char default_type = 0) const noexcept -> char {
    return _type ? _type : default_type;
  }

  auto verbose() const noexcept -> bool {
    return _prefix == '#';
  }

  auto sign(bool is_neg) const noexcept -> Str {
    if (is_neg) {
      return "-";
    }
    switch (_sign) {
      case '+': return "+";
      case '-': return " ";
      default:  return "";
    }
  }

  auto precision(u32 default_val) const noexcept -> u32 {
    return _point ? _precision : default_val;
  }

  auto prefix() const noexcept -> Str {
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
 public:
  static void fmt(bool val, auto& f) {
    f.pad(val ? Str{"true"} : Str{"false"});
  }

  static void fmt(char val, auto& f) {
    f.write_char(val);
  }

  static void fmt(const void* val, auto& f) {
    char buf[8 * sizeof(val)];
    const auto uval = reinterpret_cast<usize>(val);
    const auto type = f._style._type ? f._style._type : 'x';
    const auto sval = num::to_str(buf, uval, type);
    f.pad_num(false, sval);
  }

  static void fmt(trait::uint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 16];
    const auto sval = num::to_str(buf, val, f._style._type);
    f.pad_num(false, sval);
  }

  static void fmt(trait::sint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 16];
    const auto uval = val >= 0 ? val : 0 - val;
    const auto sval = num::to_str(buf, uval, f._style._type);
    f.pad_num(val < 0, sval);
  }

  static void fmt(trait::flt_ auto val, auto& f) {
    static constexpr auto DEFAULT_PREC = sizeof(val) == 4 ? 4U : 6U;
    char buf[8 * sizeof(val) + 16];
    const auto prec = f._style._point ? f._style._precision : DEFAULT_PREC;
    const auto uval = val >= 0 ? val : 0 - val;
    const auto sval = num::to_str(buf, uval, prec, f._style._type);
    f.pad_num(val < 0, sval);
  }

  static void fmt(trait::enum_ auto val, auto& f) {
    using E = decltype(val);
    if constexpr (requires { to_str(val); }) {
      f.pad(to_str(val));
    } else {
      f.write_fmt("{}({})", str::type_name<E>(), static_cast<int>(val));
    }
  }

  template <class T, usize N>
  static void fmt(const T (&val)[N], auto& f) {
    Slice{val}.fmt(f);
  }

  template <usize N>
  static void fmt(const char (&val)[N], auto& f) {
    Str{val}.fmt(f);
  }
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
    const char v[] = {c, c, c, c, c, c, c, c};
    for (auto i = 0U; i < n; i += sizeof(v)) {
      const auto w = n - i < sizeof(v) ? n - i : sizeof(v);
      this->write_str({v, w});
    }
  }

  void pad(Str s) {
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

  void pad_num(bool is_neg, Str body) {
    const auto sign = _style.sign(is_neg);
    const auto prefix = _style.prefix();
    const auto align = (_style._prefix || _style._fill == '0') ? '=' : _style._align;
    const auto npad = _style._width > body._len ? _style._width - body._len : 0U;

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
    } else if constexpr (requires { Debug::fmt(val, *this); }) {
      Debug::fmt(val, *this);
    } else {
      static_assert(false, "Fmter::write_val: unsupported type");
    }
  }

  void write_fmt(Str fmts, const auto&... args) {
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

  void debug_begin(Str s) noexcept {
    this->write_str(s);
    _depth += 1;
  }

  void debug_end(Str s, u32 cnt) noexcept {
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

  auto entry(Str name, const auto& value) -> DebugMap& {
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

  auto field(Str name, const auto& value) -> DebugStruct& {
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

void write(auto& out, Str fmts, const auto&... args) {
  Fmter{out}.write_fmt(fmts, args...);
}

}  // namespace sfc::fmt
