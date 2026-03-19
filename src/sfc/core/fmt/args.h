#pragma once

#include "sfc/core/trait.h"

namespace sfc::fmt {

struct RawStr {
  const char* _ptr;
  usize _len;

  template <usize N>
  consteval static auto from(const char (&s)[N]) -> RawStr {
    return {s, N - 1};
  };

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
        const auto ch = _str._ptr[_pos++];
        if (!(ch >= '0' && ch <= '9')) break;
        res = res * 10 + static_cast<u32>(ch - '0');
      }
      return res;
    };
  };

  // [[fill]align][sign]['#'][0][width][.][precision][type]
  constexpr static auto from(RawStr s) noexcept -> Spec {
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

    res._sign = parser.extract('+', '-', ' ');
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

template <class... T>
struct Fmts {
  static constexpr auto N = sizeof...(T);
  RawStr _str;
  Spec _specs[N];
  u32 _fills[N + 1][2];

 public:
  consteval Fmts(const auto& s) : _str{RawStr::from(s)} {
    auto p = 0U;
    for (auto i = 0U; i < N; ++i) {
      const auto a = _str.find('{', p);
      const auto b = _str.find('}', a);
      if (b >= _str._len) {
        throw "sfc::fmt::Fmts: not enough format args!";
      }
      _fills[i][0] = static_cast<u32>(p);
      _fills[i][1] = static_cast<u32>(a - p);
      _specs[i] = Spec::from({_str._ptr + a, b - a});
      p = b + 1;
    }
    if (_str.find('{', p) != _str._len) {
      throw "sfc::fmt::Fmts: too many format args!";
    }

    _fills[N][0] = p;
    _fills[N][1] = _str._len - p;
  }

  void fmt_args(auto& f, const T&... args) {
    auto idx = 0U;
    (void)(this->fmt_imp(f, idx++, args), ...);
    f.write_str({_str._ptr + _fills[N][0], _fills[N][1]});
  }

  void fmt_imp(auto& f, u32 idx, const auto& arg) const {
    if (idx >= N) return;
    f.write_str({_str._ptr + _fills[idx][0], _fills[idx][1]});
    f.write_arg(_specs[idx], arg);
  }
};

template <>
struct Fmts<> {
  RawStr _str;
  consteval Fmts(const auto& s) : _str{RawStr::from(s)} {}

  void fmt_args(auto& f) const {
    f.write_str({_str._ptr, _str._len});
  }
};

#ifdef __INTELLISENSE__
template <class... T>
using fmts_t = Fmts<T...>;
#else
template <class... T>
using fmts_t = trait::identity_t<Fmts<T...>>;
#endif

}  // namespace sfc::fmt
