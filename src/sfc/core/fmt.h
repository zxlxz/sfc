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

template <class T>
struct Display : T {
  Display() = delete;
  ~Display() = delete;
  using T::fmt;
};

template <class... T>
struct Args;

template <class W>
class Fmter {
  W& _out;
  Style _style = {};
  int _depth = 0;

 public:
  explicit Fmter(W& out) : _out{out} {}

  auto style() const -> const Style& {
    return _style;
  }

  void set_style(Style style) {
    _style = style;
  }

  void write_char(char c) {
    if constexpr (requires { _out.write_char(c); }) {
      _out.write_char(c);
    } else {
      _out.write_str({&c, 1});
    }
  }

  void write_str(str::Str s) {
    if (s.is_empty()) {
      return;
    }
    _out.write_str(s);
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
      case '>':
        pad_fill(npad);
        _out.write_str(s);
        break;
      case '<':
        _out.write_str(s);
        pad_fill(npad);
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
    const auto width = _style.width();
    const auto sign = _style.sign(is_neg);
    const auto prefix = _style.prefix();
    const auto fill = _style.fill(_style._prefix ? '0' : ' ');
    const auto align = fill == '0' ? '=' : _style.align();
    const auto len = prefix.len() + sign.len() + body.len();
    const auto npad = width > len ? width - len : 0U;

    auto pad_fill = [&](usize n) {
      for (auto i = 0U; i < n; ++i) {
        _out.write_str({&fill, 1});
      }
    };

    switch (align) {
      default:
      case '>':
        pad_fill(npad);
        this->write_str(sign);
        this->write_str(prefix);
        this->write_str(body);
        break;
      case '<':
        this->write_str(sign);
        this->write_str(prefix);
        this->write_str(body);
        pad_fill(npad);
        break;
      case '=':
        _out.write_str(sign);
        _out.write_str(prefix);
        pad_fill(npad);
        _out.write_str(body);
        break;
      case '^':
        pad_fill((npad + 0) / 2);
        _out.write_str(sign);
        _out.write_str(prefix);
        _out.write_str(body);
        pad_fill((npad + 1) / 2);
    }
  }

  template <class T>
  void write_val(const T& val) {
    if constexpr (requires { val.fmt(*this); }) {
      val.fmt(*this);
    } else {
      static_cast<const Display<T>*>(static_cast<const void*>(&val))->fmt(*this);
    }
  }

  void write_fmt(str::Str fmts, const auto&... args) {
    if constexpr (sizeof...(args) == 0) {
      this->write_str(fmts);
    } else {
      Args{fmts, args...}.fmt(*this);
    }
  }

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
  void debug_begin(str::Str s) {
    this->write_str(s);
    _depth += 1;
  }

  void debug_end(str::Str s, u32 cnt) {
    _depth -= 1;

    if (cnt && _style._prefix == '#') {
      this->write_char('\n');
      for (auto i = 0; i < _depth; ++i) {
        this->write_str("  ");
      }
    }
    this->write_str(s);
  }

  void debug_idx(u32 idx) {
    if (idx != 0) {
      this->write_char(',');
    }
    if (_style._prefix != '#') {
      this->write_char(' ');
    } else {
      this->write_char('\n');
      for (auto i = 0; i < _depth; ++i) {
        this->write_str("  ");
      }
    }
  }

  void debug_val(const auto& val) {
    const auto old_style = _style;
    _style = Style{._type = _style._type};
    this->write_val(val);
    _style = old_style;
  }
};

template <class W>
class Fmter<W>::DebugTuple {
  Fmter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugTuple(Fmter& fmt) : _fmt{fmt} {
    _fmt.debug_begin("(");
  }

  ~DebugTuple() {
    _fmt.debug_end(")", _cnt);
  }

  DebugTuple(const DebugTuple&) = delete;

  void entry(const auto& value) {
    _fmt.debug_idx(_cnt++);
    _fmt.debug_val(value);
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
  explicit DebugList(Fmter& fmt) : _fmt{fmt} {
    _fmt.debug_begin("[");
  }

  ~DebugList() {
    _fmt.debug_end("]", _cnt);
  }

  DebugList(const DebugList&) noexcept = delete;

  void entry(const auto& value) {
    _fmt.debug_idx(_cnt++);
    _fmt.debug_val(value);
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
  explicit DebugSet(Fmter& fmt) : _fmt{fmt} {
    _fmt.debug_begin("{");
  }

  ~DebugSet() {
    _fmt.debug_end("}", _cnt);
  }

  DebugSet(const DebugSet&) noexcept = delete;

  void entry(const auto& value) {
    _fmt.debug_idx(_cnt++);
    _fmt.debug_val(value);
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
  explicit DebugMap(Fmter& fmt) : _fmt{fmt} {
    _fmt.debug_begin("{");
  }

  ~DebugMap() {
    _fmt.debug_end("}", _cnt);
  }

  DebugMap(const DebugMap&) noexcept = delete;

  void entry(str::Str name, const auto& value) {
    _fmt.debug_idx(_cnt++);
    _fmt.write_str("\"");
    _fmt.write_str(name);
    _fmt.write_str("\": ");
    _fmt.debug_val(value);
  }

  void entries(auto&& iter) {
    iter.for_each([&](auto&& item) {
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
  explicit DebugStruct(Fmter& fmt) : _fmt{fmt} {
    _fmt.debug_begin("{");
  }

  ~DebugStruct() {
    _fmt.debug_end("}", _cnt);
  }

  DebugStruct(DebugStruct&&) noexcept = delete;

  auto field(str::Str name, const auto& value) -> DebugStruct& {
    _fmt.debug_idx(_cnt++);
    _fmt.write_str(name);
    _fmt.write_str(": ");
    _fmt.debug_val(value);
    return *this;
  }

  void fields(auto&& iter) {
    iter.for_each([&](auto&& item) {
      const auto& [k, v] = item;
      this->field(k, v);
    });
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

      f.set_style(Style::from_str(ss).unwrap_or({}));
      f.write_val(*ptr);
    });
    f.write_str(pats);
  }
};

template <>
struct Display<bool> {
  bool _val;

 public:
  void fmt(auto& f) const {
    f.pad(_val ? str::Str{"true"} : str::Str{"false"});
  }
};

template <>
struct Display<char> {
  char _val;

 public:
  void fmt(auto& f) const {
    f.write_str(str::Str{&_val, 1});
  }
};

template <trait::int_ T>
struct Display<T> {
  T _val;

 public:
  void fmt(auto& f) const {
    char buf[32];
    const auto s = this->fill(buf, f.style());
    f.pad_num(_val < 0, s);
  }

  auto fill(slice::Slice<char> buf, const Style& style) const -> str::Str;
};

template <trait::float_ T>
struct Display<T> {
  T _val;

 public:
  void fmt(auto& f) const {
    char buf[8 * sizeof(T)];
    const auto nums = this->fill(buf, f.style());
    f.pad_num(_val < 0, nums);
  }

  auto fill(slice::Slice<char> buf, const Style& style) const -> str::Str;
};

template <>
struct Display<const void*> {
  const void* _val;

 public:
  void fmt(auto& f) const {
    char buf[8 * sizeof(_val)];
    const auto nums = this->fill(buf, f.style());
    f.pad_num(false, nums);
  }

  auto fill(slice::Slice<char> buf, const Style& style) const -> str::Str;
};

template <class T>
struct Display<T*> {
  T* _val;

 public:
  void fmt(auto& f) const {
    Display<const void*>{_val}.fmt(f);
  }
};

template <class T, usize N>
struct Display<T[N]> {
  T _val[N];

 public:
  void fmt(auto& f) const {
    auto imp = f.debug_list();
    for (auto& x : _val) {
      imp.entry(x);
    }
  }
};

template <usize N>
struct Display<char[N]> {
  char _val[N];

  void fmt(auto& f) const {
    f.pad(_val);
  }
};

template <trait::enum_ T>
struct Display<T> {
  using U = __underlying_type(T);
  T _val;

 public:
  void fmt(auto& f) const {
    if constexpr (requires { to_str(_val); }) {
      const auto s = to_str(_val);
      f.pad(s);
    } else {
      const auto underly_val = static_cast<U>(_val);
      f.write_fmt("{}({})", str::type_name<T>(), underly_val);
    }
  }
};

void write(auto& out, str::Str fmts, const auto&... args) {
  Fmter{out}.write_fmt(fmts, args...);
}

}  // namespace sfc::fmt
