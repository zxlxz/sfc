#include "sfc/core/fmt.h"

namespace sfc::fmt {

struct FltBuf {
  char* _start;
  char* _end;
  char* _ptr = _end;

 public:
  auto as_str() const noexcept -> Str {
    const auto len = static_cast<usize>(_end - _ptr);
    return {_ptr, len};
  }

  void fill(auto val, u32 precision, char type) noexcept {
    const auto uval = val >= 0 ? val : -val;
    switch (type | 32) {
      default:
      case 'f': this->write_fix(uval, precision); break;
      case 'e': this->write_exp(uval, precision); break;
    }
    if (val < 0) {
      this->push('-');
    }
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
    if (_ptr == _start) {
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

auto Debug::to_str(Slice<char> buf, trait::flt_ auto val, Style style) noexcept -> Str {
  static constexpr auto DEFAULT_PRECISION = sizeof(val) == 4 ? 4U : 6U;

  if (__builtin_isnan(val)) {
    return "nan";
  }

  if (__builtin_isinf(val)) {
    return val > 0.0f ? Str{"inf"} : Str{"-inf"};
  }

  const auto prec = style.precision(DEFAULT_PRECISION);
  auto ss = FltBuf{buf._ptr, buf._ptr + buf._len};
  ss.fill(val, prec, style.type());
  return ss.as_str();
}

template auto Debug::to_str(Slice<char>, f32, Style) noexcept -> Str;
template auto Debug::to_str(Slice<char>, f64, Style) noexcept -> Str;

}  // namespace sfc::fmt
