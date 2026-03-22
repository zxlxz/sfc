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

  template <class S>
  [[gnu::always_inline]] operator S() const noexcept {
    return S{_ptr, _len};
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

template <class X>
struct Args {
  X _imp;

 public:
  void fmt(auto& f) const {
    _imp(f);
  }
};

template <class... T>
struct Fmts {
  static constexpr auto N = sizeof...(T);
  RawStr _str;
  Spec _specs[N];
  RawStr _fills[N + 1];

 public:
  consteval Fmts(const auto& s) : _str{RawStr::from(s)} {
    auto p = 0U;
    for (auto i = 0U; i < N; ++i) {
      const auto a = _str.find('{', p);
      const auto b = _str.find('}', a);
      if (b >= _str._len) {
        throw "sfc::fmt::Fmts: not enough format args!";
      }
      _fills[i] = {_str._ptr + p, a - p};
      _specs[i] = Spec::from({_str._ptr + a + 1, b - a - 1});
      p = b + 1;
    }
    if (_str.find('{', p) != _str._len) {
      throw "sfc::fmt::Fmts: too many format args!";
    }

    _fills[N] = {_str._ptr + p, _str._len - p};
  }

  auto bind(const T&... args) const {
    auto fmt = [&, *this](auto& f) {
      auto idx = 0U;
      (void)((f.write_str(_fills[idx]), f.write_arg(_specs[idx], args), ++idx), ...);
      f.write_str(_fills[N]);
    };
    return Args{fmt};
  }
};

template <>
struct Fmts<> {
  RawStr _str;
  consteval Fmts(const auto& s) : _str{RawStr::from(s)} {}
};

template <class... T>
using fmts_t = trait::identity_t<Fmts<T...>>;

}  // namespace sfc::fmt
