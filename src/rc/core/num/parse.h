#pragma once

#include "rc/core/num/mod.h"
#include "rc/core/str.h"

namespace rc::num {

template<class T>
struct ParseResult {
  T _val;
  Str _rem;

  auto val() const noexcept -> Option<T> {
    if (_rem.is_empty()) return {_val};
    return {};
  }
};

template<class T, class S>
ParseResult(const T&, const S&) -> ParseResult<T>;

struct Parser {
  const u8* _ptr;
  const u8* _end;

  explicit constexpr Parser(const Str& s) noexcept
      : _ptr{s._inner._ptr}, _end{s._inner._ptr + s._inner._len} {}

  auto is_empty() const -> bool {
    return _ptr == _end; 
  }

  auto extract_sign() noexcept -> u8 {
    const auto c = *_ptr;
    if (c == '+' || c == '-') ++_ptr;
    return c;
  }

  auto extract_digits() noexcept -> Slice<const u8> {
    const auto p = _ptr;
    while (_ptr < _end && u8'0' <= *_ptr && *_ptr <= u8'9') ++_ptr;
    return Slice<const u8>{p, usize(_ptr - p)};
  }

  auto extract_frac() -> Slice<const u8> {
    if (*_ptr != u8'.') return Slice<const u8>{};
    ++_ptr;
    return this->extract_digits();
  }

  auto extract_exp() -> Slice<const u8> {
    if (*_ptr != u8'e' && *_ptr != u8'E') return Slice<const u8>{};
    ++_ptr;
    return this->extract_digits();
  }

  template <class T>
  static auto dec2int(const Slice<const u8>& s) -> T {
    T res = 0;

    auto p = s._ptr;
    while (p < s._ptr + s._len) {
      const auto n = *p++ - u8'0';
      res = T(res * 10 + n);
    }
    return res;
  }

  template <class T>
  static auto dec2flt(const Slice<const u8>& sint, const Slice<const u8>& sfrac,
                      const Slice<const u8>& sexp) -> T {
    const auto fint = Parser::dec2int<u64>(sint);
    const auto frac = Parser::dec2int<u64>(sfrac);
    const auto fexp = Parser::dec2int<i64>(sexp);

    auto base = fint;
    auto xexp = fexp - i64(sfrac._len);
    auto frac_len = sfrac._len;
    while (frac_len > 0) base = base * 10;
    base += frac;

    if (xexp == 0) return T(base);

    auto scal = i64(1);
    for (auto uexp = fexp < 0 ? -fexp : fexp; uexp > 0; --uexp) {
      scal *= 10;
    }
    return xexp > 0 ? T(base) * T(scal) : T(base) / T(scal);
  }

  
  template <class T>
  auto parse_int() -> ParseResult<T> {
    if (_ptr == _end) return ParseResult{T(0), ""};

    if constexpr (rc::is_unsigned<T>()) {
      const auto digits = this->extract_digits();
      const auto fval = Parser::dec2int<T>(digits);
      return ParseResult{fval , Str{_ptr, usize(_end - _ptr)}};
    } else {
      const auto sign = this->extract_sign();
      const auto digits = this->extract_digits();
      const auto uval = Parser::dec2int<T>(digits);
      const auto fval = sign == u8'-' ? -uval : +uval;
      return ParseResult{T(fval), Str{_ptr, usize(_end - _ptr)}};
    }
  }

  template <class T>
  auto parse_flt() -> ParseResult<T> {
    if (_ptr == _end) return ParseResult{T(0), ""};

    const auto sign = this->extract_sign();
    const auto sint = this->extract_digits();
    const auto frac = this->extract_frac();
    const auto sexp = this->extract_exp();
    const auto uval = Parser::dec2flt<T>(sint, frac, sexp);
    const auto fval = sign == u8'-' ? -uval : +uval;
    return ParseResult{fval, Str{_ptr, usize(_end - _ptr)}};
  }
};

}  // namespace rc::num

namespace rc::str {

template <>
struct FromStr<u8> {
  static auto from_str(const Str& s) noexcept -> Option<u8> {
    return num::Parser{s}.parse_int<u8>().val();
  }
};

template <>
struct FromStr<i8> {
  static auto from_str(const Str& s) noexcept -> Option<i8> {
    return num::Parser{s}.parse_int<i8>().val();
  }
};

template <>
struct FromStr<u16> {
  static auto from_str(const Str& s) noexcept -> Option<u16> {
    return num::Parser{s}.parse_int<u16>().val();
  }
};

template <>
struct FromStr<i16> {
  static auto from_str(const Str& s) noexcept -> Option<i16> {
    return num::Parser{s}.parse_int<i16>().val();
  }
};

template <>
struct FromStr<u32> {
  static auto from_str(const Str& s) noexcept -> Option<u32> {
    return num::Parser{s}.parse_int<u32>().val();
  }
};

template <>
struct FromStr<i32> {
  static auto from_str(const Str& s) noexcept -> Option<i32> {
    return num::Parser{s}.parse_int<i32>().val();
  }
};

template <>
struct FromStr<u64> {
  static auto from_str(const Str& s) noexcept -> Option<u64> {
    return num::Parser{s}.parse_int<u64>().val();
  }
};

template <>
struct FromStr<i64> {
  static auto from_str(const Str& s) noexcept -> Option<i64> {
    return num::Parser{s}.parse_int<i64>().val();
  }
};

template <>
struct FromStr<f32> {
  static auto from_str(const Str& s) noexcept -> Option<f32> {
    return num::Parser{s}.parse_flt<f32>().val();
  }
};

template <>
struct FromStr<f64> {
  static auto from_str(const Str& s) noexcept -> Option<i64> {
    return num::Parser{s}.parse_flt<i64>().val();
  }
};

}  // namespace rc::str
