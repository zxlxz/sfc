#include "sfc/core/num.h"
#include "sfc/core/str.h"

namespace sfc::num {

struct IntStr {
  const char* _ptr;
  const char* _end;

 public:
  template <class T>
  auto parse() -> option::Option<T> {
    if (_ptr == _end) {
      return {};
    }

    const auto sign = this->extract_sign();
    if constexpr (is_unsigned<T>()) {
      if (sign == '-') {
        return {};
      }
    }

    const auto radix = this->extract_radix();
    if (_ptr == _end) {
      return {};
    }

    const auto uval = this->extract_int<T>(radix);
    if (_ptr != _end) {
      return {};
    }

    if constexpr (is_signed<T>()) {
      if (sign == '-') {
        return -uval;
      }
    }

    return uval;
  }

 private:
  auto extract_sign() -> char {
    if (_ptr == _end) {
      return 0;
    }

    const auto c = *_ptr;
    if (c == '+' || c == '-') {
      ++_ptr;
    }
    return c;
  }

  auto extract_radix() -> u32 {
    if (_ptr == _end) {
      return 10;
    }

    const auto c = *_ptr;
    const auto d = *(_ptr + 1);
    if (c != '0') {
      return 10;
    }
    ++_ptr;

    switch (d) {
      case 'b':
      case 'B': ++_ptr; return 2;
      case 'x':
      case 'X': ++_ptr; return 16;
      default:  return 8;
    }
    return 10;
  }

  template <class T>
  auto extract_int(u32 radix) -> T {
    static constexpr auto MAX_VALUE = num::max_value<T>();

    auto res = T{0} + 0U;

    for (; _ptr != _end; ++_ptr) {
      const auto c = *_ptr;
      const auto n = c <= '9' ? u32(c - '0') : u32((c | 32) - 'a');
      if (n >= radix) {
        break;
      }
      const auto t = radix * res + n;
      if (t < res || t > MAX_VALUE) {
        break;
      }
      res = t;
    }

    return static_cast<T>(res);
  }
};

struct IntBuf {
  char* _start;
  char* _end;
  char* _ptr = _end;

 public:
  auto as_str() const -> str::Str {
    return {_ptr, static_cast<usize>(_end - _ptr)};
  }

  void fill(auto val, char type) {
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
  }

 private:
  void push(char c) {
    if (_ptr == _start) {
      return;
    }
    *--_ptr = c;
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
    const auto MASK = RADIX - 1;
    const auto DIGITS = UC ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           : "0123456789abcdefghijklmnopqrstuvwxyz";

    if (val == 0) {
      return this->push('0');
    }

    for (; val != 0; val /= RADIX) {
      this->push(DIGITS[val & MASK]);
    }
  }
};

auto int2str(slice::Slice<char> buf, auto val, char type) -> str::Str {
  auto imp = IntBuf{buf._ptr, buf._ptr + buf._len};
  imp.fill(val, type);
  return imp.as_str();
}

template auto int2str(slice::Slice<char> buf, signed char val, char type) -> str::Str;
template auto int2str(slice::Slice<char> buf, short val, char type) -> str::Str;
template auto int2str(slice::Slice<char> buf, int val, char type) -> str::Str;
template auto int2str(slice::Slice<char> buf, long val, char type) -> str::Str;
template auto int2str(slice::Slice<char> buf, long long val, char type) -> str::Str;

template auto int2str(slice::Slice<char> buf, unsigned char val, char type) -> str::Str;
template auto int2str(slice::Slice<char> buf, unsigned short val, char type) -> str::Str;
template auto int2str(slice::Slice<char> buf, unsigned int val, char type) -> str::Str;
template auto int2str(slice::Slice<char> buf, unsigned long val, char type) -> str::Str;
template auto int2str(slice::Slice<char> buf, unsigned long long val, char type) -> str::Str;

}  // namespace sfc::num

namespace sfc::str {

template <class T>
auto FromStr<T>::from_str(Str s) -> option::Option<T> {
  auto imp = num::IntStr{s._ptr, s._ptr + s._len};
  return imp.parse<T>();
}

template struct FromStr<signed char>;
template struct FromStr<signed short>;
template struct FromStr<signed int>;
template struct FromStr<signed long>;
template struct FromStr<signed long long>;

template struct FromStr<unsigned char>;
template struct FromStr<unsigned short>;
template struct FromStr<unsigned int>;
template struct FromStr<unsigned long>;
template struct FromStr<unsigned long long>;

}  // namespace sfc::str
