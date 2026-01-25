#pragma once

#include "sfc/core/fmt/debug.h"
#include "sfc/core/fmt/args.h"

namespace sfc::fmt {

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
