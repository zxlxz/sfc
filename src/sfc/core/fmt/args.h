#pragma once

#include "sfc/core/mod.h"

namespace sfc::fmt {

struct RawStr {
  const char* _ptr = nullptr;
  usize _len = 0;

 public:
  consteval RawStr() = default;

  consteval RawStr(const char* p, usize n) : _ptr{p}, _len{n} {}

  template <usize N>
  consteval RawStr(const char (&s)[N]) : _ptr{s}, _len{N - 1} {}

  consteval auto operator[](u32 idx) const -> char {
    return idx < _len ? _ptr[idx] : '\0';
  }

  consteval auto find(char c, u32 pos = 0) const -> u32 {
    for (; pos < _len; ++pos) {
      if (_ptr[pos] == c) {
        break;
      }
    }
    return pos;
  }
};

struct Options {
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
  consteval static auto from(RawStr s) noexcept -> Options {
    auto idx = 0U;
    auto extract = [p = s._ptr, n = s._len, &idx](auto... c) -> char {
      if (idx < n && (sizeof...(c) == 0 || ((p[idx] == c) || ...))) {
        return p[idx++];
      }
      return '\0';
    };

    auto extract_u32 = [p = s._ptr, n = s._len, &idx]() -> u32 {
      auto res = 0U;
      for (; idx < n; idx += 1) {
        if (p[idx] < '0' || p[idx] > '9') break;
        res = res * 10 + static_cast<u32>(p[idx] - '0');
      }
      return res;
    };

    auto res = Options{};
    extract(':');

    res._fill = extract();
    res._align = extract('<', '>', '=', '^');
    if (res._fill && !res._align) {
      idx -= 1;
      res._fill = 0;
      res._align = extract('<', '>', '=', '^');
    }

    res._sign = extract('+', '-', ' ');
    res._prefix = extract('#');
    if (!res._fill) {
      res._fill = extract('0');
    }

    res._width = static_cast<u8>(extract_u32());
    if ((res._point = extract('.'))) {
      res._precision = static_cast<u8>(extract_u32());
    }
    res._type = extract();
    return res;
  }

  auto type(char default_type = 0) const -> char {
    return _type ? _type : default_type;
  }

  auto precision(u32 default_prec = 0) const -> u32 {
    return _point ? _precision : default_prec;
  }
};

struct Fmts {
  static constexpr auto N = 16U;
  RawStr _str = {};
  Options _options[N] = {};
  RawStr _pieces[N + 1] = {};

 public:
  consteval Fmts(const auto& s) : _str{s} {
    auto p = 0U;
    for (auto i = 0U; i < N; ++i) {
      const auto a = _str.find('{', p);
      const auto b = _str.find('}', a);
      _pieces[i] = RawStr{_str._ptr + p, a - p};
      if (a == _str._len || b == _str._len) {
        break;
      }
      _options[i] = Options::from({_str._ptr + (a + 1), b - a - 1});
      p = b + 1;
    }
  }
};

template <class... T>
struct Args {
  static constexpr auto N = sizeof...(T);
  RawStr _fmts;
  Options _options[N] = {};
  RawStr _pieces[N + 1] = {};
  const void* _args[sizeof...(T)];

 public:
  explicit Args(Fmts fmts, const T&... args) : _fmts{fmts._str}, _args{&args...} {
    for (auto I = 0U; I < N; ++I) {
      _options[I] = fmts._options[I];
      _pieces[I] = fmts._pieces[I];
    }
    _pieces[N] = fmts._pieces[N];
  }

  void fmt(auto& f) const noexcept {
    this->fmt_imp<0U, T...>(f);
    if (_pieces[N]._len != 0) {
      f.write_str({_pieces[N]._ptr, _pieces[N]._len});
    }
  }

 private:
  template <u32 I, class U, class... S>
  void fmt_imp(auto& f) const {
    const auto& x = *static_cast<const U*>(_args[I]);
    if (const auto s = _pieces[I]; s._len != 0) {
      f.write_str({s._ptr, s._len});
    }
    f._options = _options[I];
    f.write_val(x);
    if constexpr (sizeof...(S)) {
      this->fmt_imp<I + 1, S...>(f);
    }
  }
};

template <>
struct Args<> {
  RawStr _fmts;

 public:
  explicit Args(Fmts fmts) : _fmts{fmts._str} {}

  void fmt(auto& f) const noexcept {
    f.pad({_fmts._ptr, _fmts._len});
  }
};

template <class... T>
Args(const auto&, const T&...) -> Args<T...>;

}  // namespace sfc::fmt
