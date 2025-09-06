#include "sfc/core/num.h"
#include "sfc/core/str.h"

namespace sfc::str {

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
    if constexpr (T{0} - T{1} > 0) {
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

    if constexpr (T{0} - T{1} < 0) {
      if (sign == '-') {
        return -uval;
      }
    }

    return uval;
  }

 private:
  auto extract_sign() -> char {
    if (_ptr + 1 >= _end) {
      return 0;
    }

    const auto c = *_ptr;
    if (c == '+' || c == '-') {
      ++_ptr;
    }
    return c;
  }

  auto extract_radix() -> u32 {
    if (_ptr + 1 >= _end) {
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
      case 'B':
        ++_ptr;
        return 2;
      case 'x':
      case 'X':
        ++_ptr;
        return 16;
      default:
        return 8;
    }
  }

  template <class T>
  auto extract_int(u32 radix) -> T {
    auto res = T{0} + 0U;

    for (; _ptr != _end; ++_ptr) {
      const auto c = *_ptr;
      const auto n = c <= '9' ? u32(c - '0') : u32((c | 32) - ('a' - 10));
      if (n >= radix) {
        break;
      }
      const auto tmp = static_cast<u32>(radix * res + n);
      if (tmp < res) {
        break;
      }
      res = tmp;
    }

    return static_cast<T>(res);
  }
};

template <class T>
auto FromStr<T>::from_str(Str s) -> option::Option<T> {
  auto imp = IntStr{s._ptr, s._ptr + s._len};
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
