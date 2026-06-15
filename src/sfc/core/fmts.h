#pragma once

#include "sfc/core/tuple.h"

namespace sfc::fmt {

void write_fmt(auto& out, const auto& args);

struct SBuf;

struct RawStr {
  const char* _ptr;
  usize _len;

  consteval RawStr() : _ptr{nullptr}, _len{0} {}
  consteval RawStr(const char* s) : _ptr{s}, _len{__builtin_strlen(s)} {}
  constexpr RawStr(const char* p, usize n) : _ptr{p}, _len{n} {}

  consteval auto find(char c, u32 pos) const -> u32 {
    while (pos < _len && _ptr[pos] != c) {
      pos++;
    }
    return pos;
  }

 public:
  void fmt(auto& f) const {
    f.pad({_ptr, _len});
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

 public:
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
    }
  };

  // [[fill]align][sign]['#'][0][width][.][precision][type]
  consteval static auto from(RawStr s) noexcept -> Spec {
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
};

struct Fmts {
  static constexpr auto kMaxLen = 16U;
  RawStr _str;
  usize _cnt = 0;
  RawStr _fills[kMaxLen] = {};
  Spec _specs[kMaxLen] = {};
  RawStr _tail = {};

 public:
  consteval Fmts(const char* s) noexcept : _str{s} {
#if !defined(__INTELLISENSE__) && !defined(__clang_analyzer__)
    auto p = 0U;
    for (_cnt = 0U; _cnt < kMaxLen; ++_cnt) {
      const auto a = _str.find('{', p);
      const auto b = _str.find('}', a);
      if (b == _str._len) break;
      _fills[_cnt] = RawStr{_str._ptr + p, a - p};
      _specs[_cnt] = Spec::from({_str._ptr + a + 1, b - a - 1});
      p = b + 1;
    }
    _tail = RawStr{_str._ptr + p, _str._len - p};
#endif
  }
};

template <class... T>
struct Args {
  Fmts _fmts;
  Tuple<const T&...> _args;

 public:
  Args(const Fmts& fmts, const T&... args) : _fmts{fmts}, _args{args...} {}

  void fmt(auto& f) const {
#if !defined(__INTELLISENSE__) && !defined(__clang_analyzer__)
    _args.map([&, idx = 0U](const auto& val) mutable {
      const auto fill = _fmts._fills[idx];
      const auto spec = _fmts._specs[idx];
      f.write_str({fill._ptr, fill._len});
      f.write_arg(spec, val);
      ++idx;
    });

    const auto s = _fmts._tail;
    f.write_str({s._ptr, s._len});
#endif
  }
};

template <class... T>
Args(const Fmts& fmts, const T&... args) -> Args<T...>;

struct XArgs {
  void (*_fmt)(const XArgs& self, SBuf& out);
  const void* _args;

 public:
  template <class... T>
  XArgs(const fmt::Args<T...>& args)
      : _fmt{[](const XArgs& self, SBuf& out) { fmt::write_fmt(out, *ptr::cast<const fmt::Args<T...>>(self._args)); }}
      , _args{&args} {}

  void fmt(auto& out) const {
    if (_fmt) _fmt(*this, out);
  }
};

}  // namespace sfc::fmt
