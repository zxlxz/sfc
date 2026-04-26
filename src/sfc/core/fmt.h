#pragma once

#include "sfc/core/reflect.h"
#include "sfc/core/fmts.h"

namespace sfc::fmt {

struct Display {
  static auto format_int(Slice<char> buf, auto val, char type = 0) -> Str;
  static auto format_ptr(Slice<char> buf, auto val, char type = 0) -> Str;
  static auto format_flt(Slice<char> buf, auto val, u32 precision = 6, char type = 0) -> Str;

  static void fmt(bool val, auto& f) {
    f.write_str(val ? Str{"true"} : Str{"false"});
  }

  static void fmt(char val, auto& f) {
    f.write_char(val);
  }

  static void fmt(const void* val, auto& f) {
    char buf[16];
    const auto s = format_ptr(buf, val, f._spec._type);
    f.write_str(s);
  }

  static void fmt(num::uint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 8];
    const auto s = format_int(buf, val + 0, f._spec._type);
    f.pad_num(false, s);
  }

  static void fmt(num::sint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 8];
    const auto s = format_int(buf, val < 0 ? -val : val, f._spec._type);
    f.pad_num(val < 0, s);
  }

  static void fmt(num::float_ auto val, auto& f) {
    static constexpr auto DEFAULT_PRECISION = sizeof(val) == 4 ? 4U : 6U;
    char buf[8 * sizeof(val) + 16];

    const auto p = f._spec.precision(DEFAULT_PRECISION);
    const auto s = format_flt(buf, val < 0 ? -val : val, p, f._spec._type);
    f.pad_num(val < 0, s);
  }

  static void fmt(enum_ auto val, auto& f) {
    const auto s = reflect::enum_name(val);
    if (!s.is_empty()) {
      f.pad(s);
    } else {
      f.write_fmt("{}({})", reflect::type_name<decltype(val)>(), static_cast<i64>(val));
    }
  }

  template <usize N>
  static void fmt(const char (&val)[N], auto& f) {
    f.pad(val);
  }

  template <class T, usize N>
  static void fmt(const T (&val)[N], auto& f) {
    auto imp = f.debug_list();
    for (auto& element : val) {
      imp.entry(element);
    }
  }
};

template <class W>
struct Fmter {
  W& _buf;
  Spec _spec = {};
  u32 _depth = 0;

 public:
  void write_char(char c) {
    if constexpr (requires { _buf.push(c); }) {
      (void)_buf.push(c);
    } else {
      this->write_str(Str{&c, 1});
    }
  }

  void write_chars(char c, usize n) {
    static constexpr auto BUF_LEN = 8U;
    const char buf[BUF_LEN] = {c, c, c, c, c, c, c, c};
    for (auto i = 0U; i < n; i += BUF_LEN) {
      this->write_str(Str{buf, i + BUF_LEN < n ? BUF_LEN : n - i});
    }
  }

  void write_str(Str s) {
    if (s._len == 0) return;
    if constexpr (requires { _buf.write_str(""); }) {
      (void)_buf.write_str(s);
    } else {
      (void)_buf.write(s.as_bytes());
    }
  }

  template <class T>
  void write_val(const T& val) {
    if constexpr (__is_class(T)) {
      val.fmt(*this);
    } else {
      fmt::Display::fmt(val, *this);
    }
  }

  template <class T>
  void write_arg(Spec spec, const T& val) {
    _spec = spec;
    if constexpr (__is_class(T)) {
      val.fmt(*this);
    } else {
      fmt::Display::fmt(val, *this);
    }
  }

  template <class... T>
  void write_fmt(const fmt::fmts_t<T...>& fmts, const T&... args) {
    Args{fmts, args...}.fmt(*this);
  }

  void pad(Str s) {
    if (_spec._width <= s._len) {
      this->write_str(s);
      return;
    }

    const auto fill = _spec._fill ? _spec._fill : ' ';
    const auto npad = usize{_spec._width} - s._len;

    switch (_spec._align) {
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

    const auto sign = is_neg ? Str{"-"} : make_sign(_spec._sign);
    const auto prefix = _spec._prefix ? make_prefix(_spec._type) : Str{""};
    const auto align = (_spec._prefix || _spec._fill == '0') ? '=' : _spec._align;
    const auto nfill = sign._len + prefix._len + body._len;
    const auto npad = _spec._width > nfill ? _spec._width - nfill : 0U;
    const auto fill = _spec._fill ? _spec._fill : _spec._prefix ? '0' : ' ';

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
    const auto pretty = _spec._prefix == '#';
    if (idx != 0) {
      this->write_str(pretty ? Str{","} : Str{", "});
    }
    if (pretty) {
      for (auto i = 0U; i < _depth; ++i) {
        this->write_str("    ");
      }
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

  void entry(const auto& value) {
    _fmt.node_item(_cnt++);
    _fmt.write_val(value);
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

  void entry(const auto& value) {
    _fmt.node_item(_cnt++);
    _fmt.write_val(value);
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

  void entry(Str key, const auto& value) {
    _fmt.node_item(_cnt++);
    _fmt.write_str('"');
    _fmt.write_str(key);
    _fmt.write_str("\": ");
    _fmt.write_val(value);
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

  void field(Str key, const auto& value) {
    _fmt.node_item(_cnt++);
    _fmt.write_str(key);
    _fmt.write_str(": ");
    _fmt.write_val(value);
  }

  void fields(auto&& iter) {
    iter.for_each([&](const auto& item) {
      const auto& [k, v] = item;
      this->field(k, v);
    });
  }
};

template <usize N>
struct FixedBuf {
  static constexpr auto CAPACITY = N;
  char _buf[N];
  usize _len{0};

 public:
  [[gnu::always_inline]] auto as_bytes() const -> Slice<const u8> {
    return {reinterpret_cast<const u8*>(_buf), _len};
  }

  [[gnu::always_inline]] auto as_str() const -> Str {
    return {_buf, _len};
  }

  [[gnu::always_inline]] void clear() {
    _len = 0;
  }

  [[gnu::always_inline]] void write_str(Str s) {
    if (s._len == 0 || _len + s._len > CAPACITY) return;
    ptr::copy_nonoverlapping(s._ptr, _buf + _len, s._len);
    _len += s._len;
  }
};

template <class... T>
void write(auto&& out, fmt::fmts_t<T...> fmts, const T&... args) {
  Fmter{out}.write_val(fmt::Args{fmts, args...});
}

}  // namespace sfc::fmt

namespace sfc::panic {

template <class... T>
[[noreturn]] void panic_fmt(fmt::Args<T...> args, panic::SourceLoc loc) {
  auto buf = fmt::FixedBuf<1024>{};
  fmt::Fmter{buf}.write_val(args);
  const auto s = fmt::RawStr{buf._buf, buf._len};
  panic::panic_imp(s, loc);
}
}  // namespace sfc::panic
