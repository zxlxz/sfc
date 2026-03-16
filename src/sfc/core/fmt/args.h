#pragma once

#include "sfc/core/mod.h"

namespace sfc::fmt {

struct Display {
  static void fmt(const auto& self, auto& f);
};

struct RawStr {
  const char* _ptr;
  usize _len;

  template <usize N>
  consteval RawStr(const char (&s)[N]) : _ptr{s}, _len{N - 1} {}

  constexpr RawStr(const char* ptr, usize len) : _ptr{ptr}, _len{len} {}

  constexpr auto find(char c, u32 pos) const -> u32 {
    while (pos < _len && _ptr[pos] != c) {
      pos++;
    }
    return pos;
  }

  constexpr auto substr(u32 start, u32 end) const -> RawStr {
    return {_ptr + start, end - start};
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

struct Fmts {
  static constexpr auto N = 16U;
  struct Idxs {
    u32 start;
    u32 end;
  };

  RawStr _str;
  Idxs _idxs[N] = {};
  Specifier _spec[N] = {};

 public:
  consteval Fmts(const auto& s) : _str{s} {
    auto p = 0U;
    for (auto i = 0U; i < N; ++i) {
      const auto a = _str.find('{', p);
      const auto b = _str.find('}', a);
      _idxs[i] = Idxs{a, b};
      if (a != b) {
        _spec[i] = Specifier::from(_str.substr(a + 1, b));
      }
      p = b + 1;
    }
  }
};

template <class... T>
struct Args {
  static constexpr auto N = sizeof...(T);
  using Idxs = Fmts::Idxs;
  RawStr _str;
  Idxs _idxs[N] = {};
  Specifier _spec[N] = {};
  const void* _args[sizeof...(T)];

 public:
  explicit Args(Fmts fmts, const T&... args) : _str{fmts._str}, _args{&args...} {
    for (auto I = 0U; I < N; ++I) {
      _spec[I] = fmts._spec[I];
      _idxs[I] = fmts._idxs[I];
    }
  }

  void fmt(auto& f) const noexcept {
    this->fmt_imp<T...>(f, 0);
    if (auto x = _idxs[N - 1].end + 1; x < _str._len) {
      const auto s = _str.substr(x, _str._len);
      f.write_str({s._ptr, s._len});
    }
  }

 private:
  template <class U, class... S>
  void fmt_imp(auto& f, u32 I = 0) const {
    const auto ids = Idxs{I == 0 ? 0 : _idxs[I - 1].end + 1, _idxs[I].start};
    if (ids.start < ids.end) {
      const auto s = _str.substr(ids.start, ids.end);
      f.write_str({s._ptr, s._len});
    }
    f._spec = _spec[I];
    Display::fmt(*static_cast<const U*>(_args[I]), f);
    if constexpr (sizeof...(S)) {
      this->fmt_imp<S...>(f, I + 1);
    }
  }
};

template <>
struct Args<> {
  RawStr _str;

 public:
  explicit Args(Fmts fmts) : _str{fmts._str} {}

  void fmt(auto& f) const noexcept {
    f.pad({_str._ptr, _str._len});
  }
};

template <class... T>
Args(const auto&, const T&...) -> Args<T...>;

}  // namespace sfc::fmt
