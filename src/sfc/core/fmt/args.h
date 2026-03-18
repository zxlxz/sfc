#pragma once

#include "sfc/core/mod.h"

namespace sfc::fmt {

struct Display {
  static void fmt(const auto& self, auto& f);
};

struct RawStr {
  const char* _ptr;
  u32 _len;

  template <u32 N>
  constexpr RawStr(const char (&s)[N]) : _ptr{s}, _len{N - 1} {}

  constexpr RawStr(const char* ptr, u32 len) : _ptr{ptr}, _len{len} {}

  constexpr auto find(char c, u32 pos) const -> u32 {
    while (pos < _len && _ptr[pos] != c) {
      pos++;
    }
    return pos;
  }

  constexpr auto operator[](const u32 (&ids)[2]) const -> RawStr {
    const auto len = ids[1] > ids[0] ? ids[1] - ids[0] : 0U;
    return RawStr{_ptr + ids[0], len};
  }
};

struct Specifier {
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
    const char* _ptr;
    const char* _end;

    constexpr auto extract(auto... c) -> char {
      if (_ptr >= _end) {
        return 0;
      }
      if (((sizeof...(c) == 0) || ... || (*_ptr == c))) {
        return *_ptr++;
      }
      return 0;
    }

    constexpr auto extract_int() -> u32 {
      auto res = 0U;
      for (; _ptr < _end; ++_ptr) {
        if (*_ptr < '0' || *_ptr > '9') break;
        res = res * 10 + static_cast<u32>(*_ptr - '0');
      }
      return res;
    };
  };

  // [[fill]align][sign]['#'][0][width][.][precision][type]
  constexpr static auto from(RawStr s) noexcept -> Specifier {
    if (s._len == 0) {
      return {};
    }

    auto parser = Parser{s._ptr, s._ptr + s._len};
    auto res = Specifier{};
    parser.extract(':');

    res._fill = parser.extract();
    res._align = parser.extract('<', '>', '=', '^');
    if (res._fill && !res._align) {
      parser._ptr -= 1;
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
    res._type = parser.extract();
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
struct Fmts_ {
  static constexpr auto N = sizeof...(T);
  RawStr _str;
  u32 _fills[N + 1][2];
  Specifier _specs[N];

 public:
  consteval Fmts_(const auto& s) : _str{s} {
    _fills[0][0] = 0;
    _fills[N][1] = _str._len;
    for (auto i = 0U; i < N; ++i) {
      const auto a = _str.find('{', _fills[i][0]);
      const auto b = _str.find('}', a);
      const auto spec = Specifier::from(_str[{a + 1, b}]);
      _fills[i + 0][1] = a;
      _fills[i + 1][0] = b + 1;
      _specs[i] = spec;
    }
  }
};

template <>
struct Fmts_<> {
  RawStr _str;
  consteval Fmts_(const auto& s) : _str{s} {}
};

template <class T>
struct TypeIdenty {
  using Type = T;
};

template <class... T>
using Fmts = Fmts_<typename TypeIdenty<T>::Type...>;

template <class... T>
struct Args {
  Fmts<T...> _fmts;
  const void* _args[sizeof...(T)];

 public:
  explicit Args(Fmts<T...> fmts, const T&... args) : _fmts{fmts}, _args{&args...} {}

  void fmt(auto& f) const noexcept {
    this->fmt_next<T...>(f, 0);

    const auto str = _fmts._str[_fmts._fills[sizeof...(T)]];
    f.write_str({str._ptr, str._len});
  }

 private:
  template <class U, class... S>
  void fmt_next(auto& f, u32 I) const {
    const auto str = _fmts._str[_fmts._fills[I]];
    const auto& val = *static_cast<const U*>(_args[I]);

    f.write_str({str._ptr, str._len});
    f._spec = _fmts._specs[I];
    Display::fmt(val, f);

    if constexpr (sizeof...(S)) {
      this->fmt_next<S...>(f, I + 1);
    }
  }
};

template <>
struct Args<> {
  RawStr _str;

 public:
  explicit Args(Fmts<> fmts) : _str{fmts._str} {}

  void fmt(auto& f) const noexcept {
    f.pad({_str._ptr, _str._len});
  }
};

template <class... T>
Args(const auto&, const T&...) -> Args<T...>;

}  // namespace sfc::fmt
