#pragma once

#include "sfc/core/mod.h"

namespace sfc::fmt {

struct RawStr {
  const char* _ptr;
  usize _len;

  constexpr RawStr(const char* p, usize n) : _ptr{p}, _len{n} {}
  consteval RawStr(const char* s) : _ptr{s}, _len{__builtin_strlen(s)} {}

  [[gnu::always_inline]] constexpr auto find(char c, u32 pos) const -> u32 {
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
  u32 _locs[N + 1] = {0};
  u32 _lens[N + 1] = {0};
  Spec _specs[N] = {};

 public:
  consteval Fmts(const char* s) : _str{s} {
    auto p = 0U;
    for (auto i = 0U; i < N; ++i) {
      const auto a = _str.find('{', p);
      const auto b = _str.find('}', a);
      if (b == _str._len) {
        throw "sfc::fmt::Fmts: not enough format specs!";
      }
      const auto f = Spec::from({_str._ptr + a + 1, b - a - 1});
      _lens[i + 0] = a - p;
      _locs[i + 1] = b + 1;
      _specs[i] = f;
      p = b + 1;
    }
    _locs[N] = p;
    _lens[N] = static_cast<u32>(_str._len - p);
    if (_str.find('{', p) != _str._len) {
      throw "sfc::fmt::Fmts: too many format specs!";
    }
  }
};

template <>
struct Fmts<0> {
  RawStr _str;
  consteval Fmts(const char* s) : _str{s} {}
};

#if defined(__INTELLISENSE__) || defined(__clang_analyzer__)
template <class... T>
using fmts_t = RawStr;
#else
template <class... T>
using fmts_t = Fmts<sizeof...(T)>;
#endif

template <class... T>
struct Args {
  static constexpr auto N = sizeof...(T);
  const fmts_t<T...>& _fmts;
  const void* _args[N];

 public:
  Args(const fmts_t<T...>& fmts, const T&... args) : _fmts{fmts}, _args{&args...} {}

  void fmt(auto& f) const {
#if !defined(__INTELLISENSE__) && !defined(__clang_analyzer__)
    const auto s = _fmts._str;
    const auto g = [&](u32 i, auto& x) {
      const auto loc = _fmts._locs[i];
      const auto len = _fmts._lens[i];
      if (len != 0) f.write_str({s._ptr + loc, len});
      f.write_arg(_fmts._specs[i], x);
    };

    auto i = 0U;
    ((g(i, *static_cast<const T*>(_args[i])), i++), ...);
    f.write_str({s._ptr + _fmts._locs[N], s._len - _fmts._locs[N]});
#endif
  }
};

template <>
struct Args<> {
  const fmts_t<>& _fmts;

 public:
  void fmt(auto& f) const {
    f.write_str({_fmts._str._ptr, _fmts._str._len});
  }
};

template <class... T>
Args(const auto&, const T&...) -> Args<T...>;

template <class... T>
void write(auto&& out, fmts_t<T...> fmts, const T&... args);

}  // namespace sfc::fmt
