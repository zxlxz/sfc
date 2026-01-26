#pragma once

#include "sfc/core/trait.h"

namespace sfc::fmt {

struct RawStr {
  const char* _ptr;
  usize _len;

  template <class S>
  operator S() const noexcept {
    return S{_ptr, _len};
  }
};

struct Style {
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
  constexpr static auto from(RawStr s) noexcept -> Style {
    static constexpr auto is_num = [](char c) { return '0' <= c && c <= '9'; };
    static constexpr auto is_any = [](char x, auto... t) { return ((x == t) || ...); };

    if (s._len == 0 || s._len > 64) {
      return {};
    }

    auto p = s._ptr;
    auto e = s._ptr + s._len;

    auto res = Style{};
    if (p < e && *p == ':') {
      p++;
    }

    if (p + 1 < e && is_any(p[1], '<', '>', '=', '^')) {
      res._fill = *p++;
      res._align = *p++;
    } else if (p < e && is_any(p[0], '<', '>', '=', '^')) {
      res._align = *p++;
    }

    if (p < e && is_any(p[0], '+', '-', ' ')) {
      res._sign = *p++;
    }

    if (p < e && p[0] == '#') {
      res._prefix = *p++;
    }

    if (p < e && p[0] == '0' && res._fill == 0) {
      res._fill = *p++;
    }

    while (p < e && is_num(*p)) {
      res._width = static_cast<u8>(res._width * 10 + (*p++ - '0'));
    }

    if (p < e && p[0] == '.') {
      res._point = *p++;
      while (p < e && is_num(*p)) {
        res._precision = static_cast<u8>(res._precision * 10 + (*p++ - '0'));
      }
    }

    if (p != e) {
      res._type = *p++;
    }
    return res;
  }
};

template <class... T>
struct Fmts {
  RawStr _text[sizeof...(T) + 1] = {};
  Style _style[sizeof...(T)] = {};

 public:
  template <usize N>
  consteval Fmts(const char (&s)[N]) noexcept {
    auto find_next = [&](auto p, char c) {
      while (p < s + N && *p != c) {
        p++;
      }
      return p;
    };

    auto p = s;
    for (auto i = 0U; i < sizeof...(T); ++i) {
      const auto a = find_next(p, '{');
      const auto b = find_next(a, '}');
      if (a == b) {
        break;
      }
      _text[i] = RawStr{p, static_cast<usize>(a - p)};
      _style[i] = Style::from({a + 1, static_cast<usize>(b - a - 1)});
      p = b + 1;
    }
    _text[sizeof...(T)] = {p, static_cast<usize>(s + N - p - 1)};
  }
};

template <>
struct Fmts<> {
  const char* _ptr;
  usize _len;

 public:
  template <usize N>
  Fmts(const char (&s)[N]) : _ptr{s}, _len{N - 1} {}
};

template <class... T>
struct Args {
  static constexpr u32 N = sizeof...(T);

  Fmts<T...> _argf;
  const void* _args[N];

 public:
  Args(Fmts<T...> argf, const T&... args) noexcept : _argf{argf}, _args{&args...} {}

  void fmt(auto& f) const noexcept {
    auto fmt_all = [&]<u32... I>(trait::idxs_t<I...>) { (void)(this->fmt_imp<I>(f), ...); };
    fmt_all(trait::idxs_seq_t<N>());
    f.write_str(_argf._text[N]);
  }

 private:
  template <u32 I>
  void fmt_imp(auto& f) const {
    using U = T...[I];
    f.write_str(_argf._text[I]);
    f._style = _argf._style[I];
    f.write_val(*static_cast<const U*>(_args[I]));
  }
};

template <class... T>
using fmts_t = Fmts<trait::type_t<T>...>;

template <class... T>
Args(fmts_t<T...>, const T&...) -> Args<T...>;

}  // namespace sfc::fmt
