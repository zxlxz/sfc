#pragma once

#include "sfc/core/mod.h"

namespace sfc::fmt {

struct Display {
  static void fmt(const auto& self, auto& f);
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
  constexpr static auto from(const char* p, usize n) noexcept -> Spec {
    auto parser = Parser{p, p + n};

    auto res = Spec{};
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

  cstr_t _ptr;
  usize _len;
  Idxs _idxs[N] = {};
  Spec _spec[N] = {};

 public:
  template <usize SL>
  consteval Fmts(const char (&s)[SL]) : _ptr{s}, _len{SL - 1} {
    auto find = [&](char c, u32 pos) -> u32 {
      while (pos < _len && _ptr[pos] != c) {
        pos++;
      }
      return pos;
    };

    auto p = 0U;
    for (auto i = 0U; i < N; ++i) {
      const auto a = find('{', p);
      const auto b = find('}', a);
      _idxs[i] = Idxs{a, b};
      if (a != b) {
        _spec[i] = Spec::from(_ptr + (a + 1), b - a - 1);
      }
      p = b + 1;
    }
  }
};

template <class... T>
struct Args {
  static constexpr auto N = sizeof...(T);
  using Idxs = Fmts::Idxs;
  cstr_t _ptr = nullptr;
  usize _len = 0;
  Idxs _idxs[N] = {};
  Spec _spec[N] = {};
  const void* _args[sizeof...(T)];

 public:
  explicit Args(Fmts fmts, const T&... args) : _ptr{fmts._ptr}, _len{fmts._len}, _args{&args...} {
    for (auto I = 0U; I < N; ++I) {
      _spec[I] = fmts._spec[I];
      _idxs[I] = fmts._idxs[I];
    }
  }

  void fmt(auto& f) const noexcept {
    this->fmt_imp<T...>(f, 0);
    if (auto x = _idxs[N - 1].end + 1; x < _len) {
      f.write_str({this->_ptr + x, this->_len - x});
    }
  }

 private:
  template <class U, class... S>
  void fmt_imp(auto& f, u32 I = 0) const {
    const auto s = Idxs{I == 0 ? 0 : _idxs[I - 1].end + 1, _idxs[I].start};
    if (s.start < s.end) {
      f.write_str({_ptr + s.start, s.end - s.start});
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
  cstr_t _ptr;
  usize _len;

 public:
  explicit Args(Fmts fmts) : _ptr{fmts._ptr}, _len{fmts._len} {}

  void fmt(auto& f) const noexcept {
    f.pad({_ptr, _len});
  }
};

template <class... T>
Args(const auto&, const T&...) -> Args<T...>;

}  // namespace sfc::fmt
