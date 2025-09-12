#include "sfc/core/num.h"
#include "sfc/core/str.h"

namespace sfc::num {

struct FltStr {
  const char* _ptr;
  const char* _end;

 public:
  template <class T>
  auto parse() -> option::Option<T> {
    const auto sign = this->extract_sign();
    const auto int_part = this->extract_int_part();
    const auto flt_part = this->extract_flt_part();
    if (_ptr != _end) {
      return {};
    }

    const auto val = static_cast<T>(int_part + flt_part);
    return sign == -1 ? -val : val;
  }

 private:
  auto extract_sign() -> int {
    if (_ptr == _end) {
      return 1;
    }

    const auto c = *_ptr;
    if (c == '+' || c == '-') {
      ++_ptr;
    }
    return c == '-' ? -1 : 1;
  }

  auto extract_int_part() -> f64 {
    i64 res = 0;
    for (; _ptr != _end; ++_ptr) {
      const auto c = *_ptr;
      const auto n = static_cast<u8>(c - '0');
      if (n >= 10) {
        break;
      }
      res = 10 * res + n;
    }
    return static_cast<f64>(res);
  }

  auto extract_flt_part() -> f64 {
    if (_ptr == _end || *_ptr != '.') {
      return 0;
    }
    ++_ptr;

    i64 val = 0;
    i64 exp = 1;
    for (; _ptr != _end; ++_ptr) {
      const auto c = *_ptr;
      const auto n = static_cast<u8>(c - '0');
      if (n >= 10) {
        break;
      }

      exp *= 10;
      val = 10 * val + n;
    }
    return static_cast<f64>(val) / exp;
  }
};

struct FltBuf {
  char* _start;
  char* _end;
  char* _ptr = _end;

 public:
  auto as_str() const -> str::Str {
    return {_ptr, static_cast<usize>(_end - _ptr)};
  }

  void fill(auto val, u32 precision, char type) {
    if (__builtin_isnan(val)) {
      this->write_str("nan");
    } else if (__builtin_isinf(val)) {
      this->write_str("inf");
    } else {
      const auto uval = val >= 0 ? val : -val;
      switch (type | 32) {
        default:
        case 'f': this->write_fix(uval, precision); break;
        case 'e': this->write_exp(uval, precision); break;
      }
    }

    if (val < 0) {
      this->push('-');
    }
  }

 private:
  static auto make_exp(u32 val) -> u64 {
    auto res = u64{1U};
    for (; val >= 2; val -= 2) {
      res *= 100;
    }
    for (; val >= 1; val -= 1) {
      res *= 10;
    }
    return res;
  }

  void push(char c) {
    if (_ptr == _start) {
      return;
    }
    *--_ptr = c;
  }

  template <usize N>
  void write_str(const char (&s)[N]) {
    for (auto i = N - 1; i != 0; --i) {
      *--_ptr = s[i - 1];
    }
  }

  void write_dec(auto uval) {
    static const char DIGITS[] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";

    for (; uval >= 100; uval /= 100) {
      const auto n = uval % 100 * 2;
      this->push(DIGITS[n + 1]);
      this->push(DIGITS[n + 0]);
    }
    if (uval >= 10) {
      const auto n = uval % 100 * 2;
      this->push(DIGITS[n + 1]);
      this->push(DIGITS[n + 0]);
    } else {
      this->push(static_cast<char>(uval + '0'));
    }
  }

  void write_fix(f64 uval, u32 precision) {
    auto int_part = static_cast<u64>(uval);

    if (precision != 0) {
      const auto frac_exp = make_exp(precision);
      const auto flt_temp = (uval - static_cast<f64>(int_part)) * frac_exp;

      auto flt_part = static_cast<u64>(flt_temp + 0.5);
      if (flt_part >= frac_exp) {
        int_part += 1;
        flt_part -= frac_exp;
      }
      this->write_dec(frac_exp + flt_part);
      ++_ptr;

      this->push('.');
    }

    this->write_dec(int_part);
  }

  void write_exp(f64 uval, u32 precision) {
    auto exp = 0;
    auto norm = uval;

    if (uval != 0.0) {
      for (; norm >= 10.0; ++exp) {
        norm /= 10.0;
      }
      for (; norm < 1.0; --exp) {
        norm *= 10.0;
      }
    }

    // write exp
    const auto uexp = static_cast<u32>(exp < 0 ? -exp : exp);
    this->write_dec(uexp);
    if (uexp < 10) {
      this->push('0');
    }
    this->push(exp < 0 ? '-' : '+');
    this->push('e');

    // Write mantissa
    this->write_fix(norm, precision);
  }
};

auto flt2str(slice::Slice<char> buf, auto val, u32 precision, char type) -> str::Str {
  auto imp = FltBuf{buf._ptr, buf._ptr + buf._len};
  imp.fill(val, precision, type);
  return imp.as_str();
}

template auto flt2str(slice::Slice<char> buf, float val, u32 precision, char type) -> str::Str;
template auto flt2str(slice::Slice<char> buf, double val, u32 precision, char type) -> str::Str;

auto flt_eq_ulp(f32 a, f32 b, u32 ulp) -> bool {
  if (__builtin_isnan(a) || __builtin_isnan(b)) {
    return false;
  }

  if (__builtin_isinf(a) || __builtin_isinf(b)) {
    return a == b;
  }

  const auto ia = __builtin_bit_cast(i32, a);
  const auto ib = __builtin_bit_cast(i32, b);
  if ((ia ^ ib) < 0) {
    return a == b;
  }

  const auto diff = static_cast<u32>(ia > ib ? ia - ib : ib - ia);
  return diff <= ulp;
}

auto flt_eq_ulp(f64 a, f64 b, u32 ulp) -> bool {
  if (__builtin_isnan(a) || __builtin_isnan(b)) {
    return false;
  }

  if (__builtin_isinf(a) || __builtin_isinf(b)) {
    return a == b;
  }

  const auto ia = __builtin_bit_cast(i64, a);
  const auto ib = __builtin_bit_cast(i64, b);
  if ((ia ^ ib) < 0) {
    return a == b;
  }

  const auto diff = static_cast<u32>(ia > ib ? ia - ib : ib - ia);
  return diff <= ulp;
}

}  // namespace sfc::num

namespace sfc::str {

template <class T>
auto FromStr<T>::from_str(Str s) -> option::Option<T> {
  auto imp = num::FltStr{s._ptr, s._ptr + s._len};
  return imp.parse<T>();
}

template struct FromStr<f32>;
template struct FromStr<f64>;

}  // namespace sfc::str
