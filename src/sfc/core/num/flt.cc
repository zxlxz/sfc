#include "sfc/core/num.h"
#include "sfc/core/str.h"

namespace sfc::num {

static auto pow10(u32 val) noexcept -> u64 {
  auto res = u64{1U};
  for (; val >= 2; val -= 2) {
    res *= 100;
  }
  for (; val >= 1; val -= 1) {
    res *= 10;
  }
  return res;
}

static auto exp10(i32 val) noexcept -> f64 {
  auto res = 1.0;
  for (; val >= 2; val -= 2) {
    res *= 100.0;
  }
  for (; val >= 1; val -= 1) {
    res *= 10.0;
  }
  for (; val <= -2; val += 2) {
    res /= 100.0;
  }
  for (; val <= -1; val += 1) {
    res /= 10.0;
  }
  return res;
}

struct FltFix {
  u64 _int;
  u64 _flt;

 public:
  static auto build(f64 val, u32 precision) -> FltFix {
    auto uval = val >= 0.0 ? val : -val;

    // extract integer part
    auto int_part = static_cast<u64>(uval);
    uval -= static_cast<f64>(int_part);

    // extract fractional part
    const auto frac_exp = pow10(precision);
    auto flt_part = static_cast<u64>(uval * frac_exp + 0.5);
    if (flt_part >= frac_exp) {
      flt_part -= frac_exp;
      ++int_part;
    }

    return {int_part, flt_part};
  }
};

struct FltExp {
  u64 _int;  // 12.3456 => 1
  u64 _flt;  // 12.3456 => 23456
  i32 _exp;  // 12.3456 => 2

 public:
  static auto build(f64 val, u32 precision) -> FltExp {
    auto uval = val >= 0.0 ? val : -val;

    // extract exp
    i32 exp = 0;
    if (uval != 0.0) {
      while (uval >= 10.0) {
        uval /= 10.0;
        ++exp;
      }
      while (uval < 1.0) {
        uval *= 10.0;
        --exp;
      }
    }

    // extract fix
    const auto fix = FltFix::build(uval, precision);
    return {fix._int, fix._flt, exp};
  }
};

struct FltFmter {
  char* const _buf;
  char* const _end;
  char* _ptr = _end;  // write backwards

 public:
  auto as_str() const -> Str {
    return Str{_ptr, static_cast<usize>(_end - _ptr)};
  }

  void write_fix(f64 val, u32 precision) noexcept {
    const auto u = FltFix::build(val, precision);
    if (precision != 0) {
      this->write_int(u._flt, precision);
      this->push('.');
    }
    this->write_int(u._int);

    if (val < 0.0) {
      this->push('-');
    }
  }

  void write_exp(f64 val, u32 precision, bool upcase = false) noexcept {
    const auto u = FltExp::build(val, precision);

    // write exp
    const auto uexp = u._exp >= 0 ? u._exp : -u._exp;
    this->write_int(uexp);
    if (uexp < 10) {
      this->push('0');
    }
    this->push(u._exp < 0 ? '-' : '+');
    this->push(upcase ? 'E' : 'e');

    // write flt
    if (precision != 0) {
      this->write_int(u._flt);
      this->push('.');
    }

    // write int
    this->write_int(u._int);

    // write sign
    if (val < 0.0) {
      this->push('-');
    }
  }

 private:
  void push(char c) noexcept {
    if (_ptr == _buf) {
      return;
    }
    *--_ptr = c;
  }

  void write_int(auto uval, u32 precision = 0) noexcept {
    static const char DIGITS[] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";

    auto n_written = 0U;
    for (; uval >= 100; uval /= 100) {
      const auto n = uval % 100 * 2;
      this->push(DIGITS[n + 1]);
      this->push(DIGITS[n + 0]);
      n_written += 2;
    }
    if (uval >= 10) {
      const auto n = uval % 100 * 2;
      this->push(DIGITS[n + 1]);
      this->push(DIGITS[n + 0]);
      n_written += 2;
    } else {
      this->push(static_cast<char>(uval + '0'));
      n_written += 1;
    }

    for (; n_written < precision; ++n_written) {
      this->push('0');
    }
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
    const auto exp_part = this->extract_exp_part();

    const auto val = (int_part + flt_part) * exp10(exp_part);
    dst = static_cast<T>(is_neg ? -val : val);
    return _ptr == _end;
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

  auto extract_exp_part() noexcept -> i32 {
    if (_end - _ptr < 2) {
      return 0;
    }
    if (*_ptr != 'e' && *_ptr != 'E') {
      return 0;
    }
    ++_ptr;

    const auto is_neg = this->extract_sign();
    i32 exp_val = 0;
    for (; _ptr != _end; ++_ptr) {
      const auto c = *_ptr;
      const auto n = static_cast<u8>(c - '0');
      if (n >= 10) {
        break;
      }
      exp_val = 10 * exp_val + n;
    }
    return is_neg ? -exp_val : exp_val;
  }
};

auto flt_eq_ulp(f64 a, f64 b, u32 ulp) noexcept -> bool {
  if (__builtin_isnan(a) || __builtin_isnan(b)) {
    return false;
  }

  if (__builtin_isinf(a) || __builtin_isinf(b)) {
    return false;
  }

  const auto ia = __builtin_bit_cast(i64, a);
  const auto ib = __builtin_bit_cast(i64, b);
  if ((ia ^ ib) < 0) {
    return a == b;
  }

  const auto diff = static_cast<u32>(ia > ib ? ia - ib : ib - ia);
  return diff <= ulp;
}

auto flt_to_str(auto val, Slice<char> buf, u32 precision, char type) noexcept -> Str {
  if (__builtin_isnan(val)) {
    return "nan";
  }

  if (__builtin_isinf(val)) {
    return val > 0.0f ? Str{"inf"} : Str{"-inf"};
  }

  auto imp = FltFmter{buf._ptr, buf._ptr + buf._len};
  switch (type) {
    case 'e':
    case 'E': imp.write_exp(val, precision, type < 'a'); break;
    default:  imp.write_fix(val, precision); break;
  }
  return imp.as_str();
}

template <class T>
auto flt_from_str(Str buf) noexcept -> Option<T> {
  auto imp = FltParser{buf._ptr, buf._ptr + buf._len};
  auto dst = T{};
  if (!imp.read(dst)) {
    return {};
  }
  return Option{dst};
}

template auto flt_to_str(f32, Slice<char>, u32, char) noexcept -> Str;
template auto flt_to_str(f64, Slice<char>, u32, char) noexcept -> Str;

template auto flt_from_str(Str) noexcept -> Option<f32>;
template auto flt_from_str(Str) noexcept -> Option<f64>;

}  // namespace sfc::num
