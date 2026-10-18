#pragma once

#include "sfc/core/tuple.h"
#include "sfc/core/num.h"
#include "sfc/core/cmp.h"

namespace sfc::fmt {

struct FmtSegment {
  const char* _ptr;
  usize _len;

 public:
  constexpr auto find(char c, u32 pos = 0) const -> u32 {
    while (pos < _len && _ptr[pos] != c) {
      pos++;
    }
    return pos;
  }

  constexpr auto read(usize cnt) -> FmtSegment {
    const auto res = FmtSegment{_ptr, cnt};
    _ptr += cnt;
    _len -= cnt;
    return res;
  }

  constexpr auto read_to(char c, bool include = false) -> FmtSegment {
    const auto pos = this->find(c);
    if (pos == _len) {
      return {nullptr, 0};
    }
    return this->read(pos + (include ? 1 : 0));
  }
};

struct FmtOptions {
  char _fill = 0;
  char _align = 0;  // [<>^=]
  char _sign = 0;   // [+- ]
  char _alt = 0;    // [#]
  char _point = 0;  // [.]
  char _type = 0;   // [*]

  u8 _width = 0;      // max width=255
  u8 _precision = 0;  // max precision is 255

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

    template <class T = u32>
    constexpr auto extract_uint() -> T {
      auto res = u64{0};
      for (; _ptr < _end; ++_ptr) {
        const auto c = *_ptr;
        if (!(c >= '0' && c <= '9')) break;
        res = res * 10 + u32(c - '0');
      }
      return T(cmp::min(res, num::Int<T>::MAX));
    }
  };

  // [[fill]align][sign][#][0][width][.precision][type]
  consteval static auto from_str(auto s) noexcept -> FmtOptions {
    if (s._len == 0) {
      return {};
    }

    auto p = Parser{s._ptr, s._ptr + s._len};
    p.extract(':');

    auto res = FmtOptions{};
    res._fill = p.pop();
    res._align = p.extract('<', '>', '=', '^');
    if (res._fill && !res._align) {
      p._ptr -= 1;
      res._fill = 0;
      res._align = p.extract('<', '>', '=', '^');
    }

    res._sign = p.extract('+', '-');
    res._alt = p.extract('#');
    if (!res._fill) {
      res._fill = p.extract('0');
    }

    res._width = p.extract_uint<u8>();
    if ((res._point = p.extract('.'))) {
      res._precision = p.extract_uint<u8>();
    }
    res._type = p.pop();
    return res;
  }
};

struct Fmts {
  static constexpr auto kMaxLen = 16U;

  usize _cnt = 0;
  FmtSegment _segment[kMaxLen + 1] = {};
  FmtOptions _options[kMaxLen] = {};

 public:
  consteval Fmts(const char* p) noexcept {
#if !defined(__INTELLISENSE__) && !defined(__clang_analyzer__)
    auto s = FmtSegment{p, __builtin_strlen(p)};
    for (_cnt = 0U; _cnt < kMaxLen; ++_cnt) {
      const auto seg = s.read_to('{');
      const auto opt = s.read_to('}', true);
      if (opt._len == 0 || opt._ptr[0] != '{') break;
      _segment[_cnt] = seg;
      _options[_cnt] = FmtOptions::from_str(FmtSegment{opt._ptr + 1, opt._len - 2});
    }
    _segment[_cnt] = s;
#endif
  }
};

struct Args {
  static constexpr auto kMaxLen = 16U;
  Fmts _fmts;
  const void* _args[kMaxLen] = {};
  void (*_fmt)(const Args& self, fmt::Formatter& f);

 public:
  Args(const Fmts& fmts, const auto&... args);

  void fmt(fmt::Formatter& f) const {
    _fmt(*this, f);
  }
};

}  // namespace sfc::fmt
