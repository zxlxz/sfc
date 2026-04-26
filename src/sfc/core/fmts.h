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
    if (pos >= _len) throw "sfc::fmt::RawStr: char not found!";
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
  struct Parser {
    RawStr _str;
    usize _pos = 0;

    constexpr auto match(auto... c) const -> bool {
      if (_pos >= _str._len) return false;
      return ((_str._ptr[_pos] == c) || ...);
    }

    constexpr auto pop() -> char {
      if (_pos >= _str._len) return 0;
      return _str._ptr[_pos++];
    }

    constexpr auto extract(auto... c) -> char {
      return this->match(c...) ? this->pop() : 0;
    }

    constexpr auto extract_int() -> u32 {
      auto res = 0U;
      while (_pos < _str._len) {
        const auto ch = _str._ptr[_pos];
        if (!(ch >= '0' && ch <= '9')) break;
        res = res * 10 + static_cast<u32>(ch - '0');
        ++_pos;
      }
      return res;
    };
  };

  // [[fill]align][sign]['#'][0][width][.][precision][type]
  consteval static auto from(RawStr s) noexcept -> Spec {
    if (s._len == 0) {
      return {};
    }

    auto parser = Parser{s, 0};
    auto res = Spec{};
    parser.extract(':');

    res._fill = parser.pop();
    res._align = parser.extract('<', '>', '=', '^');
    if (res._fill && !res._align) {
      parser._pos -= 1;
      res._fill = 0;
      res._align = parser.extract('<', '>', '=', '^');
    }

    res._sign = parser.extract('+', '-');
    res._prefix = parser.extract('#');
    if (!res._fill) {
      res._fill = parser.extract('0');
    }

    res._width = static_cast<u8>(parser.extract_int());
    if ((res._point = parser.extract('.'))) {
      res._precision = static_cast<u8>(parser.extract_int());
    }
    res._type = parser.pop();
    return res;
  }

  auto type(char default_type = 0) const -> char {
    return _type ? _type : default_type;
  }

  auto precision(u32 default_prec = 0) const -> u32 {
    return _point ? _precision : default_prec;
  }
};

template <u32 N>
struct Fmts {
  RawStr _str;
  u32 _idxs[N] = {};
  u32 _ends[N] = {};
  Spec _specs[N] = {};

 public:
  consteval Fmts(const char* s) noexcept : _str{s} {
#if !defined(__INTELLISENSE__) && !defined(__clang_analyzer__)
    for (auto i = 0U; i < N; ++i) {
      const auto a = _str.find('{', i == 0 ? 0U : _ends[i - 1]);
      const auto b = _str.find('}', a);
      _idxs[i] = a;
      _ends[i] = b + 1;
      _specs[i] = Spec::from({_str._ptr + a + 1, b - a - 1});
    }
#endif
  }

  struct Item {
    RawStr fill;
    Spec spec;
  };
  auto operator[](u32 idx) const -> Item {
    if (idx >= N) return {"", {}};
    const auto i = idx == 0 ? 0 : _ends[idx - 1];
    const auto e = _idxs[idx];
    const auto s = RawStr{_str._ptr + i, e - i};
    return {s, _specs[idx]};
  }

  auto tail() const -> RawStr {
    const auto idx = _ends[N - 1];
    return {_str._ptr + idx, _str._len - idx};
  }
};

template <>
struct Fmts<0> {
  RawStr _str;
  consteval Fmts(const char* s) : _str{s} {}
};

#if defined(__INTELLISENSE__) || defined(__clang_analyzer__)
// make IntelliSense and clang-tidy happy
template <class... T>
using fmts_t = Fmts<0>;
#else
template <class... T>
using fmts_t = Fmts<sizeof...(T)>;
#endif

template <class... T>
struct Args {
  fmts_t<T...> _fmts;
  Tuple<const T&...> _args;

 public:
  Args(const fmts_t<T...>& fmts, const T&... args) : _fmts{fmts}, _args{args...} {}

  void fmt(auto& f) const {
#if !defined(__INTELLISENSE__) && !defined(__clang_analyzer__)
    _args.map([&, idx = 0U](const auto& val) mutable {
      const auto [fill, spec] = _fmts[idx];
      f.write_str({fill._ptr, fill._len});
      f.write_arg(spec, val);
      ++idx;
    });
    const auto s = _fmts.tail();
    f.write_str({s._ptr, s._len});
#endif
  }
};

template <>
struct Args<> {
  fmts_t<> _fmts;

 public:
  void fmt(auto& f) const {
#if !defined(__INTELLISENSE__) && !defined(__clang_analyzer__)
    const auto s = _fmts._str;
    f.write_str({s._ptr, s._len});
#endif
  }
};

template <class... T>
Args(const auto&, const T&...) -> Args<T...>;

}  // namespace sfc::fmt
