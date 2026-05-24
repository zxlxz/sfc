#pragma once

#include "sfc/core/tuple.h"

namespace sfc::fmt {

struct RawStr {
  const char* _ptr;
  usize _len;

  constexpr RawStr(const char* p, usize n) : _ptr{p}, _len{n} {}
  consteval RawStr(const char* s) : _ptr{s}, _len{__builtin_strlen(s)} {}

  consteval auto find(char c, u32 pos) const -> u32 {
    while (pos < _len && _ptr[pos] != c) {
      pos++;
    }
    return pos;
  }
};

struct Spec {
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
  consteval static auto from(RawStr s) noexcept -> Spec;

  auto type(char default_type = 0) const -> char {
    return _type ? _type : default_type;
  }

  auto align(char default_align = 0) const -> char {
    return _align ? _align : default_align;
  }

  auto fill(char default_type = ' ') const -> char {
    return _fill ? _fill : default_type;
  }

  auto width() const -> u32 {
    return _width;
  }

  auto precision(u32 default_prec = 0) const -> u32 {
    return _point ? _precision : default_prec;
  }

  auto sign(bool is_neg) const -> str::Str;
  auto prefix() const -> str::Str;
};

struct Parser {
  const char* _ptr;
  const char* _end;

 public:
  constexpr auto match(auto... c) const -> bool {
    if (_ptr >= _end) return false;
    return ((*_ptr == c) || ...);
  }

  constexpr auto pop() -> char {
    if (_ptr >= _end) return 0;
    return *_ptr++;
  }

  constexpr auto extract(auto... c) -> char {
    return this->match(c...) ? this->pop() : 0;
  }

  constexpr auto extract_int() -> u32 {
    auto res = 0U;
    for (; _ptr < _end; ++_ptr) {
      const auto c = *_ptr;
      if (!(c >= '0' && c <= '9')) break;
      const auto n = c - '0';
      res = res * 10 + num::cast_unsigned(n);
    }
    return res;
  };
};

// [[fill]align][sign]['#'][0][width][.][precision][type]
consteval auto Spec::from(RawStr s) noexcept -> Spec {
  if (s._len == 0) {
    return {};
  }

  auto res = Spec{};

  auto p = Parser{s._ptr, s._ptr + s._len};
  p.extract(':');

  res._fill = p.pop();
  res._align = p.extract('<', '>', '=', '^');
  if (res._fill && !res._align) {
    p._ptr -= 1;
    res._fill = 0;
    res._align = p.extract('<', '>', '=', '^');
  }

  res._sign = p.extract('+', '-');
  res._prefix = p.extract('#');
  if (!res._fill) {
    res._fill = p.extract('0');
  }

  res._width = num::saturating_cast<u8>(p.extract_int());
  if ((res._point = p.extract('.'))) {
    res._precision = num::saturating_cast<u8>(p.extract_int());
  }
  res._type = p.pop();
  return res;
}

struct Fmts {
  static constexpr auto kMaxLen = 16U;
  RawStr _str;
  u32 _cnt = 0;
  u32 _idxs[kMaxLen] = {};
  u32 _ends[kMaxLen] = {};
  Spec _specs[kMaxLen] = {};

 public:
  consteval Fmts(const char* s) noexcept : _str{s} {
#if !defined(__INTELLISENSE__) && !defined(__clang_analyzer__)
    auto p = 0U;
    for (auto i = 0U; i < kMaxLen; ++i) {
      const auto a = _str.find('{', p);
      const auto b = _str.find('}', a);
      if (b == _str._len) break;
      _idxs[_cnt] = a;
      _ends[_cnt] = b;
      _specs[_cnt] = Spec::from({_str._ptr + a + 1, b - a - 1});
      p = b + 1;
      _cnt += 1;
    }
#endif
  }

  struct Item {
    RawStr _fill;
    Spec _spec;
  };
  auto operator[](u32 idx) const -> Item {
    if (idx >= _cnt) return {"", {}};
    const auto a = idx == 0 ? 0UL : _ends[idx - 1] + 1;
    const auto b = _idxs[idx];
    const auto s = RawStr{_str._ptr + a, b - a};
    return {s, _specs[idx]};
  }

  auto tail() const -> RawStr {
    const auto i = _cnt == 0 ? 0U : _ends[_cnt - 1] + 1;
    return {_str._ptr + i, _str._len - i};
  }
};

}  // namespace sfc::fmt
