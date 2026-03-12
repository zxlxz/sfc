#pragma once

#include "sfc/core/fmt/args.h"
#include "sfc/core/fmt/display.h"

namespace sfc::fmt {

template <class W>
struct Fmter {
  W& _buf;
  Specifier _spec = {};
  u32 _depth = 0;

 public:
  void write_char(char c) {
    if constexpr (requires { _buf.push(c); }) {
      (void)_buf.push(c);
    } else {
      (void)_buf.write_str(Str{&c, 1});
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
    if constexpr (requires { _buf.push_str(s); }) {
      (void)_buf.push_str(s);
    } else {
      (void)_buf.write_str(s);
    }
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

  void write_fmt(Fmts fmts, const auto&... args) {
    Args{fmts, args...}.fmt(*this);
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
    Display::fmt(value, _fmt);
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
    Display::fmt(value, _fmt);
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
    Display::fmt(value, _fmt);
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
    Display::fmt(value, _fmt);
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
    Display::fmt(value, _fmt);
  }

  void fields(auto&& iter) {
    iter.for_each([&](const auto& item) {
      const auto& [k, v] = item;
      this->field(k, v);
    });
  }
};

struct Buffer {
  using buf_t = u8*;
  const buf_t _buf;
  const usize _cap;
  usize _len = 0;

 public:
  auto as_bytes() const -> Slice<const u8> {
    return {_buf, _len};
  }

  void write_str(Str s) {
    if (s._len == 0 || _len + s._len > _cap) return;
    __builtin_memcpy(_buf + _len, s._ptr, s._len);
    _len += s._len;
  }
};

void write(auto& out, Fmts fmts, const auto&... args) {
  static_assert(requires { out.write_str(""); });
  auto fmter = Fmter{out};
  fmter.write_fmt(fmts, args...);
}

}  // namespace sfc::fmt

namespace sfc::panic {

[[noreturn]] void panic_fmt(Location loc, fmt::Fmts fmts, const auto&... args) {
  if constexpr (sizeof...(args) == 0) {
    const auto s = fmts._str;
    panic::panic_imp(loc, s._ptr, s._len);
  } else {
    u8 buf[256];
    auto out = fmt::Buffer{buf, sizeof(buf)};
    fmt::write(out, fmts, args...);
    panic::panic_imp(loc, out._buf, out._len);
  }
}

}  // namespace sfc::panic
