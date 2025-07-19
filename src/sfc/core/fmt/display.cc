#include "sfc/core/fmt/display.h"

#include "sfc/core/fmt/fmter.h"

namespace sfc::fmt {

using slice::Slice;
using str::Str;

struct NumBuf {
  char* _ptr;
  usize _len;
  char* _end = _ptr + _len;

 public:
  auto as_str() const -> str::Str {
    return {_ptr, static_cast<usize>(_end - _ptr)};
  }

  auto write_int(auto val, char type) -> NumBuf& {
    const auto uval = val > 0 ? val : 0 - val;

    switch (type) {
      default:  this->write_dec(uval); break;
      case 'B': this->write_bin<2>(uval); break;
      case 'b': this->write_bin<2>(uval); break;
      case 'O': this->write_bin<8>(uval); break;
      case 'o': this->write_bin<8>(uval); break;
      case 'X': this->write_bin<16, true>(uval); break;
      case 'x': this->write_bin<16, false>(uval); break;
    }

    if (val != uval) {
      this->push('-');
    }
    return *this;
  }

  auto write_flt(auto val, u32 precision, char type) -> NumBuf& {
    if (__builtin_isnan(val)) {
      this->write_str("nan");
      return *this;
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

    return *this;
  }

 private:
  static auto make_exp(u32 val) -> u64 {
    auto res = u64{1U};
    for (; val > 2; val -= 2) {
      res *= 100;
    }
    for (; val > 1; val -= 1) {
      val *= 10;
    }
    return res;
  }

  void push(char c) {
    *--_end = c;
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

  template <u32 RADIX, bool UC = false>
  void write_bin(auto val) {
    const auto MASK   = RADIX - 1;
    const auto DIGITS = UC ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           : "0123456789abcdefghijklmnopqrstuvwxyz";

    if (val == 0) {
      return this->push('0');
    }

    for (; val != 0; val /= RADIX) {
      this->push(DIGITS[val & MASK]);
    }
  }

  void write_fix(f64 val, u32 precision) {
    auto int_part = static_cast<u64>(val);

    if (precision != 0) {
      const auto frac_exp = make_exp(precision);
      const auto tmp_part = (val - static_cast<f64>(int_part)) * frac_exp;

      auto flt_part = static_cast<u64>(__builtin_round(tmp_part));
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
    auto exp  = 0;
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

auto IFmt<signed char>::to_str(slice::Slice<char> buf, char type) const -> Str {
  return NumBuf{buf._ptr, buf._len}.write_int(_val + 0, type).as_str();
}

auto IFmt<short>::to_str(slice::Slice<char> buf, char type) const -> Str {
  return NumBuf{buf._ptr, buf._len}.write_int(_val + 0, type).as_str();
}

auto IFmt<int>::to_str(slice::Slice<char> buf, char type) const -> Str {
  return NumBuf{buf._ptr, buf._len}.write_int(_val, type).as_str();
}

auto IFmt<long>::to_str(slice::Slice<char> buf, char type) const -> Str {
  return NumBuf{buf._ptr, buf._len}.write_int(_val, type).as_str();
}

auto IFmt<long long>::to_str(slice::Slice<char> buf, char type) const -> Str {
  return NumBuf{buf._ptr, buf._len}.write_int(_val, type).as_str();
}

auto IFmt<unsigned char>::to_str(slice::Slice<char> buf, char type) const -> Str {
  return NumBuf{buf._ptr, buf._len}.write_int(_val + 0U, type).as_str();
}

auto IFmt<unsigned short>::to_str(slice::Slice<char> buf, char type) const -> Str {
  return NumBuf{buf._ptr, buf._len}.write_int(_val + 0U, type).as_str();
}

auto IFmt<unsigned int>::to_str(slice::Slice<char> buf, char type) const -> Str {
  return NumBuf{buf._ptr, buf._len}.write_int(_val, type).as_str();
}

auto IFmt<unsigned long>::to_str(slice::Slice<char> buf, char type) const -> Str {
  return NumBuf{buf._ptr, buf._len}.write_int(_val, type).as_str();
}

auto IFmt<unsigned long long>::to_str(slice::Slice<char> buf, char type) const -> Str {
  return NumBuf{buf._ptr, buf._len}.write_int(_val, type).as_str();
}

auto IFmt<float>::to_str(slice::Slice<char> buf, u32 precision, char type) const -> Str {
  return NumBuf{buf._ptr, buf._len}.write_flt(_val, precision, type).as_str();
}

auto IFmt<double>::to_str(slice::Slice<char> buf, u32 precision, char type) const -> Str {
  return NumBuf{buf._ptr, buf._len}.write_flt(_val, precision, type).as_str();
}

}  // namespace sfc::fmt
