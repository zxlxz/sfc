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
    const auto type = f._style._type ? f._style._type : 'p';
    const auto sval = num::int_to_str(uval, buf, type);
    f.pad_num(false, sval);
  }

  static void fmt(trait::uint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 16];
    const auto sval = num::int_to_str(val, buf, f._style._type);
    f.pad_num(false, sval);
  }

  static void fmt(trait::sint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 16];
    const auto uval = val >= 0 ? val : 0 - val;
    const auto sval = num::int_to_str(uval, buf, f._style._type);
    f.pad_num(val < 0, sval);
  }

  static void fmt(trait::flt_ auto val, auto& f) {
    static constexpr auto DEFAULT_PREC = sizeof(val) == 4 ? 4U : 6U;
    char buf[8 * sizeof(val) + 16];
    const auto prec = f._style._point ? f._style._precision : DEFAULT_PREC;
    const auto uval = val >= 0 ? val : 0 - val;
    const auto sval = num::flt_to_str(uval, buf, prec, f._style._type);
    f.pad_num(val < 0, sval);
  }

  template <trait::enum_ E>
  static void fmt(E val, auto& f) {
    if constexpr (requires { E::_COUNT_; }) {
      const auto s = Debug::enum_name(val);
      if (!s.is_empty()) {
        return f.pad(s);
      }
    }
    char buf[8];
    const auto sval = num::int_to_str(static_cast<u32>(val), buf);
    f.pad(sval);
  }

  template <class T, usize N>
  static void fmt(const T (&val)[N], auto& f) {
    if constexpr (trait::same_<T, char>) {
      Str{val}.fmt(f);
    } else {
      Slice{val, N}.fmt(f);
    }
  }

 private:
  template <class T>
  static constexpr Str type_name() {
    constexpr auto S1 = sizeof("Str sfc::str::type_name() [T =");
    constexpr auto S2 = sizeof("]");
    return Str{__PRETTY_FUNCTION__ + S1, sizeof(__PRETTY_FUNCTION__) - S1 - S2};
  }

  template <auto E>
  static constexpr Str enum_name() {
    static constexpr auto SN = str::type_name<decltype(E)>();
    static constexpr auto S1 = sizeof("Str sfc::str::enum_name() [E =");
    static constexpr auto S2 = sizeof("]");
    static constexpr auto ss = Str{__PRETTY_FUNCTION__ + S1, sizeof(__PRETTY_FUNCTION__) - S1 - S2};
    for (auto n = ss._len; n != 0; --n) {
      if (ss._ptr[n - 1] == ':') {
        return Str{ss._ptr + n, ss._len - n};
      }
    }
    return ss;
  }

  template <trait::enum_ E>
  static auto enum_name(E val) -> Str {
    static constexpr auto COUNT = static_cast<u32>(E::_COUNT_) & 0xFFU;
    static const auto NAMES = []<u32... I>(trait::idxs_t<I...>) {
      static const Str names[] = {str::enum_name<static_cast<E>(I)>()...};
      return names;
    }(trait::idxs_seq_t<COUNT>{});
    const auto idx = static_cast<usize>(val);
    if (idx < COUNT) {
      return NAMES[idx];
    }
    return {};
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
  u32 _depth = 0;

 public:
  void write_char(char c) {
    if constexpr (requires { _out.push(c); }) {
      _out.push(c);
    } else {
      _out.write_str({&c, 1});
    }
  }

  void write_chars(char c, usize n) {
    const char v[] = {c, c, c, c, c, c, c, c};
    for (auto i = 0U; i < n; i += sizeof(v)) {
      const auto w = n - i < sizeof(v) ? n - i : sizeof(v);
      this->write_str({v, w});
    }
  }

  void write_str(Str s) {
    if (s.is_empty()) {
      return;
    }
    if constexpr (requires { _out.push_str(s); }) {
      _out.push_str(s);
    } else {
      _out.write_str(s);
    }
  }

  void pad(Str s) {
    if (_style._width <= s._len) {
      this->write_str(s);
      return;
    }

    const auto fill = _style._fill ? _style._fill : ' ';
    const auto npad = usize{_style._width} - s._len;

    switch (_style._align) {
      default:
      case '<':
        this->write_str(s);
        this->write_chars(fill, npad);
        break;
      case '>':
        this->write_chars(fill, npad);
        this->write_str(s);
        break;
      case '=':
      case '^':
        this->write_chars(fill, (npad + 0) / 2);
        this->write_str(s);
        this->write_chars(fill, (npad + 1) / 2);
        break;
    }
  }

  void pad_num(bool is_neg, Str body) {
    auto make_sign = [](char sign) -> Str {
      switch (sign) {
        case '+': return Str{"+"};
        case '-': return Str{" "};
        case ' ': return Str{" "};
        default:  return Str{""};
      }
    };

    auto make_prefix = [](char type) -> Str {
      switch (type) {
        case 'O': return "0";
        case 'o': return "0";
        case 'B': return "0B";
        case 'b': return "0b";
        case 'X': return "0X";
        case 'x': return "0x";
        case 'P': return "0X";
        case 'p': return "0x";
        default:  return "";
      }
    };

    const auto sign = is_neg ? Str{"-"} : make_sign(_style._sign);
    const auto prefix = _style._prefix ? make_prefix(_style._type) : Str{""};
    const auto align = (_style._prefix || _style._fill == '0') ? '=' : _style._align;
    const auto nfill = sign._len + prefix._len + body._len;
    const auto npad = _style._width > nfill ? _style._width - nfill : 0U;
    const auto fill = _style._fill ? _style._fill : _style._prefix ? '0' : ' ';

    switch (align) {
      default:
      case '>':
        this->write_chars(fill, npad);
        this->write_str(sign);
        this->write_str(prefix);
        this->write_str(body);
        break;
      case '<':
        this->write_str(sign);
        this->write_str(prefix);
        this->write_str(body);
        this->write_chars(fill, npad);
        break;
      case '=':
        this->write_str(sign);
        this->write_str(prefix);
        this->write_chars(fill, npad);
        this->write_str(body);
        break;
      case '^':
        this->write_chars(fill, (npad + 0) / 2);
        this->write_str(sign);
        this->write_str(prefix);
        this->write_str(body);
        this->write_chars(fill, (npad + 1) / 2);
    }
  }

  void write_val(const auto& val) {
    if constexpr (requires { val.fmt(*this); }) {
      val.fmt(*this);
    } else {
      Debug::fmt(val, *this);
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
  struct DebugTuple;
  auto debug_tuple() -> DebugTuple {
    return DebugTuple{*this};
  }

  struct DebugList;
  auto debug_list() -> DebugList {
    return DebugList{*this};
  }

  struct DebugSet;
  auto debug_set() -> DebugSet {
    return DebugSet{*this};
  }

  struct DebugMap;
  auto debug_map() -> DebugMap {
    return DebugMap{*this};
  }

  struct DebugStruct;
  auto debug_struct() -> DebugStruct {
    return DebugStruct{*this};
  }

 private:
  static constexpr auto INDENT_SIZE = 4U;
  static constexpr auto MAX_DEPTH = 20U;
  static constexpr auto PRETTY_STR =
      "\n"
      "                                                            "
      "                                                            ";

  void node_begin(Str s) noexcept {
    this->write_str(s);
    _depth += 1;
  }

  void node_end(Str s, u32 cnt) noexcept {
    _depth -= 1;
    if (cnt != 0) {
      this->node_item(0);
    }
    this->write_str(s);
  }

  void node_item(u32 idx) {
    const auto pretty = _style._prefix == '#';
    if (idx != 0) {
      this->write_str(pretty ? Str{","} : Str{", "});
    }
    if (pretty) {
      this->write_str({PRETTY_STR, 1 + INDENT_SIZE * _depth});
    }
  }
};

template <class W>
struct Fmter<W>::DebugTuple {
  Fmter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugTuple(Fmter& fmt) : _fmt{fmt} {
    _fmt.node_begin("(");
  }

  ~DebugTuple() {
    _fmt.node_end(")", _cnt);
  }

  DebugTuple(const DebugTuple&) = delete;
  void operator=(const DebugTuple&) = delete;

  auto entry(const auto& value) -> DebugTuple& {
    _fmt.node_item(_cnt++);
    _fmt.write_val(value);
    return *this;
  }

  void entries(auto&& iter) {
    iter.for_each([&](auto&& val) { this->entry(val); });
  }
};

template <class W>
struct Fmter<W>::DebugList {
  Fmter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugList(Fmter& fmt) : _fmt{fmt} {
    _fmt.node_begin("[");
  }

  ~DebugList() {
    _fmt.node_end("]", _cnt);
  }

  DebugList(const DebugList&) = delete;
  void operator=(const DebugList&) = delete;

  auto entry(const auto& value) -> DebugList& {
    _fmt.node_item(_cnt++);
    _fmt.write_val(value);
    return *this;
  }

  void entries(auto&& iter) {
    iter.for_each([&](auto&& val) { this->entry(val); });
  }
};

template <class W>
struct Fmter<W>::DebugSet {
  Fmter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugSet(Fmter& fmt) : _fmt{fmt} {
    _fmt.node_begin("{");
  }

  ~DebugSet() {
    _fmt.node_end("}", _cnt);
  }

  DebugSet(const DebugSet&) = delete;
  void operator=(const DebugSet&) = delete;

  auto entry(const auto& value) -> DebugSet& {
    _fmt.node_item(_cnt++);
    _fmt.write_val(value);
    return *this;
  }

  void entries(auto&& iter) {
    iter.for_each([&](auto&& val) { this->entry(val); });
  }
};

template <class W>
struct Fmter<W>::DebugMap {
  Fmter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugMap(Fmter& fmt) : _fmt{fmt} {
    _fmt.node_begin("{");
  }

  ~DebugMap() {
    _fmt.node_end("}", _cnt);
  }

  DebugMap(const DebugMap&) = delete;
  void operator=(const DebugMap&) = delete;

  auto entry(Str key, const auto& value) -> DebugMap& {
    _fmt.node_item(_cnt++);
    _fmt.write_str('"');
    _fmt.write_str(key);
    _fmt.write_str("\": ");
    _fmt.write_val(value);
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
struct Fmter<W>::DebugStruct {
  Fmter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugStruct(Fmter& fmt) : _fmt{fmt} {
    _fmt.node_begin("{");
  }

  ~DebugStruct() {
    _fmt.node_end("}", _cnt);
  }

  DebugStruct(const DebugStruct&) = delete;
  void operator=(const DebugStruct&) = delete;

  auto field(Str key, const auto& value) -> DebugStruct& {
    _fmt.node_item(_cnt++);
    _fmt.write_str(key);
    _fmt.write_str(": ");
    _fmt.write_val(value);
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
