#include "sfc/core/num.h"
#include "sfc/core/str.h"

namespace sfc::str {

struct Str2Int {
  Slice<const char> _s;

 public:
  template <trait::UInt T>
  auto parse_int() -> Option<T> {
    const auto sign = this->extract_sign();
    if (sign == '-') {
      return {};
    }

    const auto radix = this->extract_radix();
    if (_s._len == 0) {
      return {};
    }

    const auto uval = this->extract_int<T>(radix);
    if (_s._len != 0) {
      return {};
    }

    return uval;
  }

  template <trait::SInt T>
  auto parse_int() -> Option<T> {
    const auto sign = this->extract_sign();

    const auto radix = this->extract_radix();
    if (_s._len == 0) {
      return {};
    }

    const auto uval = this->extract_int<T>(radix);
    if (_s._len != 0) {
      return {};
    }

    return sign == '-' ? static_cast<T>(-uval) : uval;
  }

 private:
  void pop() {
    _s._ptr += 1;
    _s._len -= 1;
  }

  auto extract_sign() -> char {
    const auto c = _s._len ? _s._ptr[0] : char(0);
    if (c == '+' || c == '-') {
      this->pop();
    }
    return c;
  }

  auto extract_radix() -> u32 {
    if (_s._len <= 1) {
      return 10;
    }

    const auto c = _s._ptr[0];
    const auto d = _s._ptr[1];
    if (c != '0') {
      return 10;
    }
    this->pop();

    switch (d) {
      case 'b':
      case 'B':
        this->pop();
        return 2;
      case 'x':
      case 'X':
        this->pop();
        return 16;
      default:
        return 8;
    }
    return 10;
  }

  template <class T>
  auto extract_int(u32 radix) -> T {
    static constexpr auto MAX_VALUE = num::max_value<T>();

    auto res = T{0} + 0U;

    for (; _s._len; this->pop()) {
      const auto c = _s._ptr[0];
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

template <class T>
auto Str::parse() const -> Option<T> {
  auto imp = Str2Int{{_ptr, _len}};
  return imp.parse_int<T>();
}

template auto Str::parse<signed char>() const -> Option<signed char>;
template auto Str::parse<signed short>() const -> Option<signed short>;
template auto Str::parse<signed int>() const -> Option<signed int>;
template auto Str::parse<signed long>() const -> Option<signed long>;
template auto Str::parse<signed long long>() const -> Option<signed long long>;

template auto Str::parse<unsigned char>() const -> Option<unsigned char>;
template auto Str::parse<unsigned short>() const -> Option<unsigned short>;
template auto Str::parse<unsigned int>() const -> Option<unsigned int>;
template auto Str::parse<unsigned long>() const -> Option<unsigned long>;
template auto Str::parse<unsigned long long>() const -> Option<unsigned long long>;

}  // namespace sfc::str
