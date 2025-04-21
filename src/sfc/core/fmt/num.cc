#include "sfc/core/fmt/num.h"

namespace sfc::fmt {

struct Int2Str {
  Style _style;

  slice::Slice<char> _buf;
  char* _ptr = _buf._ptr + _buf._len;

 public:
  auto as_str() const -> str::Str {
    const auto len = static_cast<usize>(_buf._ptr + _buf._len - _ptr);
    return {_ptr, len};
  }

  auto operator()(auto val) -> str::Str {
    const auto uval = val > 0 ? val : 0 - val;
    const auto radix = this->radix();
    if (radix == 10) {
      this->write_dec(uval);
    } else {
      this->write_bin(uval, radix);
      this->write_prefix();
    }

    return this->as_str();
  }

 private:
  void push(char c) {
    if (_buf._ptr == _ptr) {
      return;
    }
    *--_ptr = c;
  }

  void push_str(str::Str s) {
    for (auto i = s._len; i != 0; --i) {
      this->push(s._ptr[i - 1]);
    }
  }

  auto radix() const -> u16 {
    switch (_style._type) {
      case 'b':
      case 'B': return 2;
      case 'o':
      case 'O': return 8;
      case 'x':
      case 'X': return 16;
      default:  return 10;
    }
  }

  auto prefix() const -> str::Str {
    if (!_style._prefix) {
      return {};
    }
    switch (_style._type) {
      case 'o': return "0";
      case 'b': return "0b";
      case 'B': return "0B";
      case 'x': return "0x";
      case 'X': return "0X";
      default:  return {};
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

  void write_bin(auto val, int radix) {
    static const char DIGITS_ABC[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static const char DIGITS_abc[] = "0123456789abcdefghijklmnopqrstuvwxyz";

    const auto base = static_cast<decltype(val)>(radix);
    const auto mask = static_cast<decltype(val)>(radix - 1);
    const auto type = _style.type();
    const auto digits = type >= 'a' ? DIGITS_abc : DIGITS_ABC;

    if (val == 0) {
      this->push('0');
    } else {
      for (; val != 0; val /= base) {
        this->push(digits[val & mask]);
      }
    }
  }

  void write_prefix() {
    const auto prefix = this->prefix();

    if (_style._align == '=') {
      const auto width = _style.width();
      const auto body = this->as_str();
      const auto npad = num::saturating_sub<usize>(width, body.len() + prefix.len());
      for (auto i = 0U; i < npad; ++i) {
        this->push('0');
      }
    }

    this->push_str(prefix);
  }
};

struct Flt2Str {
  Style _style;

  slice::Slice<char> _buf;
  usize _len = 0U;

 public:
  auto as_str() const -> str::Str {
    return {_buf._ptr, _len};
  }

  auto operator()(auto val) -> str::Str {
    static const auto DEFAULT_PREC = sizeof(val) == sizeof(f32) ? 4 : 6;

    const auto uval = num::fabs(val);
    const auto prec = _style.precision(DEFAULT_PREC);
    const auto type = uval < 1e30 ? _style.type() : 'g';
    switch (type) {
      default:
      case 'f': this->printf("%.*f", prec, uval); break;
      case 'g': this->printf("%.*g", prec, uval); break;
      case 'e': this->printf("%.*e", prec, uval); break;
    }
    return this->as_str();
  }

 private:
  void printf(cstr_t fmts, const auto&... args) {
    const auto ptr = _buf.as_mut_ptr() + _len;
    const auto rem = _buf.len() - _len;
    const auto cnt = __builtin_snprintf(ptr, rem, fmts, args...);
    _len += static_cast<u32>(cnt);
  }
};

template <class T>
auto Int<T>::to_str(const Style& style, slice::Slice<char> sbuf) const -> str::Str {
  return Int2Str{style, sbuf}(_val);
}

template <class T>
auto Flt<T>::to_str(const Style& style, slice::Slice<char> sbuf) const -> str::Str {
  return Flt2Str{style, sbuf}(_val);
}

auto Ptr::to_str(const Style& style, slice::Slice<char> sbuf) const -> str::Str {
  const auto uval = mem::bit_cast<u64>(_val);

  auto imp = Int2Str{style, sbuf};
  if (!style._type) {
    imp._style._prefix = '#';
    imp._style._type = 'x';
    imp._style._width = 12;
    imp._style._fill = '0';
  }

  return imp(uval);
}

template struct Int<signed char>;
template struct Int<short>;
template struct Int<int>;
template struct Int<long>;
template struct Int<long long>;

template struct Int<unsigned char>;
template struct Int<unsigned short>;
template struct Int<unsigned int>;
template struct Int<unsigned long>;
template struct Int<unsigned long long>;

template struct Flt<float>;
template struct Flt<double>;

}  // namespace sfc::fmt
