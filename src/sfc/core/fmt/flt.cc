#include "sfc/core/fmt.h"

namespace sfc::fmt {

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

auto Debug::fill_flt(slice::Slice<char> buf, Style style, auto val) -> str::Str {
  const auto prec = style.precision(sizeof(val) <= sizeof(f32) ? 4 : 6);
  auto ss = FltBuf{buf._ptr, buf._ptr + buf._len};
  ss.fill(val, prec, style.type());
  return ss.as_str();
}

template auto Debug::fill_flt(slice::Slice<char>, Style, f32) -> str::Str;
template auto Debug::fill_flt(slice::Slice<char>, Style, f64) -> str::Str;

}  // namespace sfc::fmt
