#pragma once

#include "sfc/core/reflect.h"
#include "sfc/core/fmts.h"

namespace sfc::fmt {

struct SBuf {
  char* const _ptr;
  usize const _cap;
  usize _len = 0;

 public:
  template <usize N>
  SBuf(char (&s)[N]) : _ptr{s}, _cap{N - 1} {}

  auto as_str() const -> Str {
    return {_ptr, _len};
  }

  void clear() {
    _len = 0;
  }

  void write_str(Str s) {
    if (s._len == 0 || _len + s._len > _cap) return;
    __builtin_memcpy(_ptr + _len, s._ptr, s._len);
    _len += s._len;
  }
};

struct Debug {
  static auto format_int(Slice<char> buf, auto val, char type = 0) -> Str;
  static auto format_ptr(Slice<char> buf, auto val, char type = 0) -> Str;
  static auto format_flt(Slice<char> buf, auto val, u32 precision = 6, char type = 0) -> Str;

  static void fmt(bool val, auto& f) {
    f.write_str(val ? Str{"true"} : Str{"false"});
  }

  static void fmt(char val, auto& f) {
    f.write_char(val);
  }

  static void fmt(char16_t val, auto& f) {
    u8 u8_buf[4] = {};
    const auto u8_len = chr::utf8_encode(val, u8_buf);
    f.write_str(Str::from_utf8({u8_buf, u8_len}));
  }

  static void fmt(char32_t val, auto& f) {
    u8 u8_buf[4] = {};
    const auto u8_len = chr::utf8_encode(val, u8_buf);
    f.write_str(Str::from_utf8({u8_buf, u8_len}));
  }

  static void fmt(num::uint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 8];
    const auto s = Debug::format_int(buf, val + 0, f._spec._type);
    f.pad_num(false, s);
  }

  static void fmt(num::sint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 8];
    const auto u = num::uabs(val);
    const auto s = Debug::format_int(buf, u, f._spec._type);
    f.pad_num(val < 0, s);
  }

  static void fmt(num::float_ auto val, auto& f) {
    static constexpr auto DEFAULT_PRECISION = sizeof(val) == 4 ? 4U : 6U;
    char buf[8 * sizeof(val) + 16];

    const auto uval = val < 0 ? -val : val;
    const auto prec = f._spec.precision(DEFAULT_PRECISION);
    const auto s = Debug::format_flt(buf, uval, prec, f._spec._type);
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

  static void fmt(const void* val, auto& f) {
    char buf[16];
    const auto s = Debug::format_ptr(buf, val, f._spec._type);
    f.write_str(s);
  }
};

template <class W>
struct Formatter {
  W& _buf;
  Spec _spec = {};

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

  void write_val(const auto& val) {
#if !defined(__INTELLISENSE__) && !defined(__clang_analyzer__)
    if constexpr (requires { val.fmt(*this); }) {
      val.fmt(*this);
    } else if constexpr (requires { Str{val}; }) {
      Str{val}.fmt(*this);
    } else if constexpr (requires { Slice{val}; }) {
      Slice{val}.fmt(*this);
    } else {
      fmt::Debug::fmt(val, *this);
    }
#endif
  }

  void write_arg(Spec spec, const auto& arg) {
    const auto old_spec = _spec;
    _spec = spec;
    this->write_val(arg);
    _spec = old_spec;
  }

  void write_fmt(const fmt::Fmts& fmts, const auto&... args) {
#if !defined(__INTELLISENSE__) && !defined(__clang_analyzer__)
    const auto u = tuple::bind(args...);
    u.map([&, idx = 0U](const auto& val) mutable {
      const auto [fill, spec] = fmts[idx];
      this->write_str({fill._ptr, fill._len});
      this->write_arg(spec, val);
      ++idx;
    });

    const auto s = fmts.tail();
    this->write_str({s._ptr, s._len});
#endif
  }

  void pad(Str s) {
    if (_spec._width <= s._len) {
      this->write_str(s);
      return;
    }

    const auto fill = _spec.fill();
    const auto npad = _spec._width - s._len;

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
    const auto sign = _spec.sign(is_neg);
    const auto prefix = _spec.prefix();
    const auto nbody = sign._len + prefix._len + body._len;
    if (nbody >= _spec._width) {
      this->write_str(sign);
      this->write_str(prefix);
      this->write_str(body);
      return;
    }

    const auto npad = _spec._width - nbody;
    const auto fill = _spec.fill(_spec._prefix ? '0' : ' ');
    const auto align = _spec.align(fill == '0' ? '=' : '>');
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
  void block_open(Str s) noexcept {
    this->write_str(s);
  }

  void block_close(Str s, u32 cnt) noexcept {
    if (cnt != 0) {
      this->block_entry(0);
    }
    this->write_str(s);
  }

  void block_entry(u32 idx) {
    if (idx != 0) {
      this->write_str(", ");
    }
  }
};

template <class W>
struct Formatter<W>::DebugTuple {
  Formatter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugTuple(Formatter& fmt) : _fmt{fmt} {
    _fmt.block_open("(");
  }

  ~DebugTuple() {
    _fmt.block_close(")", _cnt);
  }

  DebugTuple(const DebugTuple&) = delete;

  auto entry(const auto& value) -> DebugTuple& {
    _fmt.block_entry(_cnt++);
    _fmt.write_val(value);
    return *this;
  }

  void entries(auto&& iter) {
    iter.for_each([&](auto&& val) { this->entry(val); });
  }
};

template <class W>
struct Formatter<W>::DebugList {
  Formatter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugList(Formatter& fmt) : _fmt{fmt} {
    _fmt.block_open("[");
  }

  ~DebugList() {
    _fmt.block_close("]", _cnt);
  }

  DebugList(const DebugList&) = delete;

  void entry(const auto& value) {
    _fmt.block_entry(_cnt++);
    _fmt.write_val(value);
  }

  void entries(auto&& iter) {
    iter.for_each([&](auto&& val) { this->entry(val); });
  }
};

template <class W>
struct Formatter<W>::DebugSet {
  Formatter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugSet(Formatter& fmt) : _fmt{fmt} {
    _fmt.block_open("{");
  }

  ~DebugSet() {
    _fmt.block_close("}", _cnt);
  }

  DebugSet(const DebugSet&) = delete;

  void entry(const auto& value) {
    _fmt.block_entry(_cnt++);
    _fmt.write_val(value);
  }

  void entries(auto&& iter) {
    iter.for_each([&](auto&& val) { this->entry(val); });
  }
};

template <class W>
struct Formatter<W>::DebugMap {
  Formatter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugMap(Formatter& fmt) : _fmt{fmt} {
    _fmt.block_open("{");
  }

  ~DebugMap() {
    _fmt.block_close("}", _cnt);
  }

  DebugMap(const DebugMap&) = delete;

  void entry(Str key, const auto& value) {
    _fmt.block_entry(_cnt++);
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
struct Formatter<W>::DebugStruct {
  Formatter& _fmt;
  u32 _cnt = 0;

 public:
  explicit DebugStruct(Formatter& fmt) : _fmt{fmt} {
    _fmt.block_open("{");
  }

  ~DebugStruct() {
    _fmt.block_close("}", _cnt);
  }

  DebugStruct(const DebugStruct&) = delete;

  void field(Str key, const auto& value) {
    _fmt.block_entry(_cnt++);
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

void write(auto&& out, const fmt::Fmts& fmts, const auto&... args) {
  Formatter{out}.write_fmt(fmts, args...);
}

}  // namespace sfc::fmt

namespace sfc::panic {
[[noreturn]] void panic_fmt(const XFmt& fmts, const auto&... args) {
  char buf[1024];
  auto out = fmt::SBuf{buf};
  fmt::write(out, fmts._fmts, args...);
  const auto s = fmt::RawStr{out._ptr, out._len};
  panic::panic_imp(s, fmts._loc);
}
}  // namespace sfc::panic
