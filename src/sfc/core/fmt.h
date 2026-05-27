#pragma once

#include "sfc/core/reflect.h"
#include "sfc/core/fmts.h"

namespace sfc::fmt {

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
    const auto u8_len = chr::utf8_encode(u8_buf, val);
    f.write_str(Str::from_utf8({u8_buf, u8_len}));
  }

  static void fmt(char32_t val, auto& f) {
    u8 u8_buf[4] = {};
    const auto u8_len = chr::utf8_encode(u8_buf, val);
    f.write_str(Str::from_utf8({u8_buf, u8_len}));
  }

  static void fmt(trait::uint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 8];
    const auto s = Debug::format_int(buf, val + 0, f._spec._type);
    f.pad_num(false, s);
  }

  static void fmt(trait::sint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 8];
    const auto u = num::unsigned_abs(val);
    const auto s = Debug::format_int(buf, u, f._spec._type);
    f.pad_num(val < 0, s);
  }

  static void fmt(trait::float_ auto val, auto& f) {
    static constexpr auto DEFAULT_PRECISION = sizeof(val) == 4 ? 4U : 6U;
    char buf[8 * sizeof(val) + 16];

    const auto uval = val < 0 ? -val : val;
    const auto prec = f._spec.precision(DEFAULT_PRECISION);
    const auto s = Debug::format_flt(buf, uval, prec, f._spec._type);
    f.pad_num(val < 0, s);
  }

  static void fmt(trait::enum_ auto val, auto& f) {
    using I = __underlying_type(decltype(val));
    constexpr auto kTypeName = reflect::type_name<decltype(val)>();
    if constexpr (requires { to_str(val); }) {
      f.write_str(to_str(val));
    } else {
      f.write_fmt(fmt::Args{"{}({})", kTypeName, __builtin_bit_cast(I, val)});
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
    this->write_str(Str{&c, 1});
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
    } else if constexpr (requires { str::Str{val}; }) {
      this->pad(Str{val});
    } else if constexpr (requires { Slice{val}; }) {
      Slice{val}.fmt(*this);
    } else {
      Debug::fmt(val, *this);
    }
  }

  template <class... T>
  void write_fmt(const fmt::Args<T...>& args) {
    args.fmt(*this);
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

  void pad_num(bool is_neg, Str num_str) {
    const auto sign = _spec.sign(is_neg);
    const auto prefix = _spec.prefix();
    if (num_str._len >= _spec._width) {
      this->write_str(sign);
      this->write_str(prefix);
      this->write_str(num_str);
      return;
    }

    const auto npad = _spec._width - num_str._len;
    const auto fill = _spec.fill(_spec._prefix ? '0' : ' ');
    const auto align = _spec.align(fill == '0' ? '=' : '>');
    switch (align) {
      default:
      case '>':
        this->write_chars(fill, npad);
        this->write_str(sign);
        this->write_str(prefix);
        this->write_str(num_str);
        break;
      case '<':
        this->write_str(sign);
        this->write_str(prefix);
        this->write_str(num_str);
        this->write_chars(fill, npad);
        break;
      case '=':
        this->write_str(sign);
        this->write_str(prefix);
        this->write_chars(fill, npad);
        this->write_str(num_str);
        break;
      case '^':
        this->write_chars(fill, (npad + 0) / 2);
        this->write_str(sign);
        this->write_str(prefix);
        this->write_str(num_str);
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

// macro: write!(out, arg...)
void write(auto&& out, const fmt::Fmts& fmts, const auto&... args) {
  Formatter{out}.write_val(fmt::Args{fmts, args...});
}

}  // namespace sfc::fmt

namespace sfc::panic {

auto PanicInfo::from_args(const auto& args, SourceLoc loc) -> PanicInfo {
  auto buf = PanicInfo::sbuf();
  fmt::Formatter{buf}.write_val(args);
  return PanicInfo{{buf._ptr, buf._len}, loc};
}

}  // namespace sfc::panic
