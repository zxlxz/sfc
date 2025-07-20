#include "sfc/core/num.h"
#include "sfc/core/str.h"

namespace sfc::num {

struct FltStr {
  const char* _ptr;
  const char* _end;

 public:
  template <class T>
  auto parse() -> Option<T> {
    const auto sign = this->extract_sign();
    const auto int_part = this->extract_int_part();
    const auto flt_part = this->extract_flt_part();
    if (_ptr != _end) {
      return {};
    }

    const auto res = sign * (int_part + flt_part);
    return Option{static_cast<T>(res)};
  }

 private:
  auto extract_sign() -> f32 {
    if (_ptr == _end) {
      return 1.0f;
    }

    const auto c = *_ptr;
    if (c == '+' || c == '-') {
      ++_ptr;
    }
    return c == '-' ? -1.0f : 1.0f;
  }

  auto extract_int_part() -> f64 {
    f64 res = 0.0;
    for (; _ptr != _end; ++_ptr) {
      const auto c = *_ptr;
      const auto n = static_cast<u8>(c - '0');
      if (n >= 10) {
        break;
      }
      res = 10 * res + n;
    }
    return res;
  }

  auto extract_flt_part() -> f64 {
    if (_ptr == _end || *_ptr != '.') {
      return 0;
    }
    ++_ptr;

    f64 res = 0.0;
    f64 exp = 1.0;
    for (; _ptr != _end; ++_ptr) {
      const auto c = *_ptr;
      const auto n = static_cast<u8>(c - '0');
      if (n >= 10) {
        break;
      }

      exp /= 10.0;
      res += n * exp;
    }
    return res;
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
      const auto uval = __builtin_fabs(val);
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
    for (auto i = N; i != 0; --i) {
      *--_end = s[i - 1];
    }
  }

  void write_dec(auto val) {
    static const char DIGITS[] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";

    for (; val >= 100; val /= 100) {
      const auto n = val % 100 * 2;
      this->push(DIGITS[n + 1]);
      this->push(DIGITS[n + 0]);
    }
    if (val >= 10) {
      const auto n = val % 100 * 2;
      this->push(DIGITS[n + 1]);
      this->push(DIGITS[n + 0]);
    } else {
      this->push(static_cast<char>(val + '0'));
    }
  }

  void write_fix(f64 val, u32 precision) {
    auto int_part = static_cast<u64>(val);

    if (precision != 0) {
      const auto frac_exp = make_exp(precision);
      const auto flt_temp = (val - static_cast<f64>(int_part)) * frac_exp;

      auto flt_part = static_cast<u64>(__builtin_round(flt_temp));
      if (flt_part >= frac_exp) {
        int_part += 1;
        flt_part -= frac_exp;
      }
      this->write_dec(flt_part);
      this->push('.');
    }

    this->write_dec(int_part);
  }

  void write_exp(f64 val, u32 precision) {
    auto exp = 0;
    auto norm = val;

    for (; norm >= 10.0; ++exp) {
      norm /= 10.0;
    }
    for (; norm < 1.0; --exp) {
      norm *= 10.0;
    }

    // Write mantissa
    this->write_fix(norm, precision);

    // write exp
    this->push(exp);
    this->push(exp < 0 ? '-' : '+');

    const auto uexp = static_cast<u32>(exp < 0 ? -exp : exp);
    this->write_dec(uexp);
    if (uexp < 10) {
      this->push('0');
    }
    if (uexp == 0) {
      this->push('0');
    }
  }
};

auto flt2str(slice::Slice<char> buf, auto val, u32 precision, char type) -> str::Str {
  auto imp = FltBuf{buf._ptr, buf._ptr + buf._len};
  imp.fill(val, precision, type);
  return imp.as_str();
}

template auto flt2str(slice::Slice<char> buf, float val, u32 precision, char type) -> str::Str;
template auto flt2str(slice::Slice<char> buf, double val, u32 precision, char type) -> str::Str;

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

  const auto diff = __builtin_llabs(ia - ib);
  return diff <= ulp;
}

}  // namespace sfc::num

namespace sfc::str {

template <class T>
auto FromStr<T>::from_str(Str s) -> Option<T> {
  auto imp = num::FltStr{s._ptr, s._ptr + s._len};
  return imp.parse<T>();
}

template struct FromStr<f32>;
template struct FromStr<f64>;

}  // namespace sfc::str
