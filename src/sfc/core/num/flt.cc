#include "sfc/core/num.h"
#include "sfc/core/str.h"

namespace sfc::num {

struct FltFmter {
  char* _buf;
  char* _ptr;

 public:
  auto write(auto val, u32 precision, char type) noexcept -> Str {
    const auto tail = _ptr;
    const auto uval = val >= 0 ? val : -val;
    switch (type | 32) {
      default:
      case 'f': this->write_fix(uval, precision); break;
      case 'e': this->write_exp(uval, precision); break;
    }
    if (val < 0) {
      this->push('-');
    }

    const auto len = static_cast<usize>(tail - _ptr);
    return {_ptr, len};
  }

 private:
  static auto make_exp(u32 val) noexcept -> u64 {
    auto res = u64{1U};
    for (; val >= 2; val -= 2) {
      res *= 100;
    }
    for (; val >= 1; val -= 1) {
      res *= 10;
    }
    return res;
  }

  void push(char c) noexcept {
    if (_ptr == _buf) {
      return;
    }
    *--_ptr = c;
  }

  void write_dec(auto uval) noexcept {
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

  void write_fix(f64 uval, u32 precision) noexcept {
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

  void write_exp(f64 uval, u32 precision) noexcept {
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

struct FltParser {
  const char* _ptr;
  const char* _end;

 public:
  template <trait::flt_ T>
  auto read(T& dst) noexcept -> bool {
    const auto is_neg = this->extract_sign();
    const auto int_part = this->extract_int_part();
    const auto flt_part = this->extract_flt_part();
    if (_ptr != _end) {
      return false;
    }

    const auto val = int_part + flt_part;
    if (_ptr != _end) {
      return false;
    }

    dst = static_cast<T>(is_neg ? -val : val);
    return true;
  }

 private:
  auto extract_sign() noexcept -> bool {
    if (_ptr == _end) {
      return false;
    }

    const auto c = *_ptr;
    if (c == '+' || c == '-') {
      ++_ptr;
    }
    return c == '-';
  }

  auto extract_int_part() noexcept -> f64 {
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

  auto extract_flt_part() noexcept -> f64 {
    if (_ptr == _end || *_ptr != '.') {
      return 0;
    }
    ++_ptr;

    i64 uval = 0;
    i64 base = 1;
    for (; _ptr != _end; ++_ptr) {
      const auto c = *_ptr;
      const auto n = static_cast<u8>(c - '0');
      if (n >= 10) {
        break;
      }

      base *= 10;
      uval = 10 * uval + n;
    }
    return static_cast<f64>(uval) / static_cast<f64>(base);
  }
};

auto flt_eq_ulp(f64 a, f64 b, u32 ulp) noexcept -> bool {
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

auto to_str(Slice<char> buf, trait::flt_ auto val, u32 precision, char type) noexcept -> Str {
  if (__builtin_isnan(val)) {
    return "nan";
  }

  if (__builtin_isinf(val)) {
    return val > 0.0f ? Str{"inf"} : Str{"-inf"};
  }

  auto imp = FltFmter{buf._ptr, buf._ptr + buf._len};
  return imp.write(val, precision, type);
}

auto from_str(str::Str buf, trait::flt_ auto& val) noexcept -> bool {
  auto imp = FltParser{buf._ptr, buf._ptr + buf._len};
  return imp.read(val);
}

template auto to_str(Slice<char>, f32, u32, char) noexcept -> Str;
template auto to_str(Slice<char>, f64, u32, char) noexcept -> Str;

template auto from_str(str::Str, f32&) noexcept -> bool;
template auto from_str(str::Str, f64&) noexcept -> bool;

}  // namespace sfc::num
