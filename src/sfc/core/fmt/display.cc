#include "sfc/core/fmt/display.h"

#include "sfc/core/fmt/fmter.h"

namespace sfc::fmt {

using slice::Slice;
using str::Str;

struct IntBuf {
  char* _ptr;
  usize _len;
  char* _end;

 public:
  auto as_str() const -> str::Str {
    return {_ptr, static_cast<usize>(_end - _ptr)};
  }

  auto write_int(auto val, const Style& style) -> IntBuf& {
    _end = _ptr + _len;

    const auto uval = val > 0 ? val : 0 - val;

    switch (style._type | 32) {
      default:
        this->write_dec(uval);
        break;
      case 'b':
        this->write_bin<2>(uval, style);
        break;
      case 'o':
        this->write_bin<8>(uval, style);
        break;
      case 'x':
        this->write_bin<16>(uval, style);
        break;
    }
  }

  auto write_ptr(auto uval, Style style) -> IntBuf& {
    _end = _ptr + _len;

    if (style._width == 0) {
      style._width = 12;
    }

    switch (style._type | 32) {
      case 'd':
      case 'n':
        this->write_dec(uval);
        break;
      case 'b':
        this->write_bin<2>(uval, style);
        break;
      case 'o':
        this->write_bin<8>(uval, style);
        break;
      default:
      case 'x':
        this->write_bin<16>(uval, style);
        break;
    }

    return *this;
  }

 private:
  void push(char c) {
    *--_end = c;
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

  template <u32 RADIX>
  void write_bin(auto val, const Style& style) {
    static constexpr auto MASK = RADIX - 1;

    if (val == 0) {
      this->push('0');
      return;
    }

    const auto digits = style._type < 'a' ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                          : "0123456789abcdefghijklmnopqrstuvwxyz";
    for (; val != 0; val /= RADIX) {
      this->push(digits[val & MASK]);
    }

    if (style._prefix == '#') {
      while (_ptr + _len - _end < style._width) {
        this->push('0');
      }

      if (RADIX != 8) {
        this->push(style._type);
      }
      this->push('0');
    }
  }
};

struct FltBuf {
  char* _ptr;
  usize _len = 0;

 public:
  auto as_str() const -> str::Str {
    return {_ptr, _len};
  }

  auto write_flt(double val, const Style& style) -> FltBuf& {
    const auto uval = num::fabs(val);

    const auto DEFAULT_PREC = sizeof(val) == sizeof(f32) ? 5U : 6U;
    const auto DEFAULT_TYPE = (1e-6 < uval && uval < 1e+6) ? 'f' : 'g';

    const auto prec = style.precision(DEFAULT_PREC);
    const auto type = style._type == 0 ? DEFAULT_TYPE : style.type();

    switch (type) {
      default:
      case 'f':
        _len = __builtin_printf(_ptr, _len, "%.*f", prec, uval);
        break;
      case 'g':
        _len = __builtin_printf(_ptr, _len, "%.*g", prec, uval);
        break;
      case 'e':
        _len = __builtin_printf(_ptr, _len, "%.*e", prec, uval);
        break;
    }

    return *this;
  }
};

auto IFmt<signed char>::to_str(const Style& style, Slice<char> buf) const -> Str {
  return IntBuf{buf._ptr, buf._len}.write_int(_val + 0, style).as_str();
}

auto IFmt<short>::to_str(const Style& style, Slice<char> buf) const -> Str {
  return IntBuf{buf._ptr, buf._len}.write_int(_val + 0, style).as_str();
}

auto IFmt<int>::to_str(const Style& style, Slice<char> buf) const -> Str {
  return IntBuf{buf._ptr, buf._len}.write_int(_val + 0, style).as_str();
}

auto IFmt<long>::to_str(const Style& style, Slice<char> buf) const -> Str {
  return IntBuf{buf._ptr, buf._len}.write_int(_val + 0, style).as_str();
}

auto IFmt<long long>::to_str(const Style& style, Slice<char> buf) const -> Str {
  return IntBuf{buf._ptr, buf._len}.write_int(_val + 0, style).as_str();
}

auto IFmt<unsigned char>::to_str(const Style& style, Slice<char> buf) const -> Str {
  return IntBuf{buf._ptr, buf._len}.write_int(_val + 0U, style).as_str();
}

auto IFmt<unsigned short>::to_str(const Style& style, Slice<char> buf) const -> Str {
  return IntBuf{buf._ptr, buf._len}.write_int(_val + 0U, style).as_str();
}

auto IFmt<unsigned int>::to_str(const Style& style, Slice<char> buf) const -> Str {
  return IntBuf{buf._ptr, buf._len}.write_int(_val + 0U, style).as_str();
}

auto IFmt<unsigned long>::to_str(const Style& style, Slice<char> buf) const -> Str {
  return IntBuf{buf._ptr, buf._len}.write_int(_val + 0U, style).as_str();
}

auto IFmt<unsigned long long>::to_str(const Style& style, Slice<char> buf) const -> Str {
  return IntBuf{buf._ptr, buf._len}.write_int(_val + 0U, style).as_str();
}

auto IFmt<float>::to_str(const Style& style, Slice<char> buf) const -> Str {
  return FltBuf{buf._ptr, buf._len}.write_flt(_val + 0.0f, style).as_str();
}

auto IFmt<double>::to_str(const Style& style, Slice<char> buf) const -> Str {
  return FltBuf{buf._ptr, buf._len}.write_flt(_val + 0.0, style).as_str();
}

auto IFmt<const void*>::to_str(const Style& style, Slice<char> buf) const -> Str {
  const auto uval = reinterpret_cast<usize>(_val);
  return IntBuf{buf._ptr, buf._len}.write_ptr(uval, style).as_str();
}

}  // namespace sfc::fmt
