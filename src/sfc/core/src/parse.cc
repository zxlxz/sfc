#include "sfc/core/str.h"

namespace sfc::str {

static auto fast_exp10(i32 cnt) -> f64 {
  static constexpr auto kLen = 16U;
  static constexpr f64 TBL[kLen] =
      {1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15};

  if (cnt == 0) {
    return 1.0;
  }

  auto res = 1.0;
  auto n = cnt > 0 ? cnt : -cnt;
  for (; n >= 256; n -= 256) {
    res *= 1e256;
  }
  for (; n >= 16; n -= 16) {
    res *= 1e16;
  }
  res *= TBL[n];
  if (cnt < 0) {
    res = 1.0 / res;
  }
  return res;
}

struct NumParser {
  const char* _ptr;
  const char* _end;

 public:
  NumParser(Str s) noexcept : _ptr{s._ptr}, _end{s._ptr + s._len} {}

  auto is_empty() const -> bool {
    return _ptr == _end;
  }

  template <class T>
  auto parse_uint() -> Option<T> {
    static constexpr auto kMaxVal = static_cast<u64>(num::max_value<T>());

    const auto sign = this->extract_sign();
    if (sign == -1) return {};

    const auto radix = this->extract_radix();
    const auto uval = radix == 10 ? this->extract_dec() : this->extract_bin(radix);
    if (uval > kMaxVal) return {};

    return static_cast<T>(uval);
  }

  template <class T>
  auto parse_sint() -> Option<T> {
    static constexpr auto MAX_VAL = static_cast<u64>(num::max_value<T>());

    const auto sign = this->extract_sign();
    const auto radix = this->extract_radix();
    const auto uval = radix == 10 ? this->extract_dec() : this->extract_bin(radix);

    if (sign == 1) {
      if (uval > MAX_VAL) return {};
      return +static_cast<T>(uval);
    } else {
      if (uval > MAX_VAL + 1) return {};
      return -static_cast<T>(uval);
    }
  }

  template <class T>
  auto parse_flt() -> Option<T> {
    const auto sign = this->extract_sign();
    const auto int_part = this->extract_dec();
    const auto flt_part = this->extract_flt();
    const auto exp_part = this->extract_exp();

    const auto uval = (static_cast<f64>(int_part) + flt_part) * exp_part;
    return static_cast<T>(sign * uval);
  }

  template <class T>
  auto parse_num() -> Option<T> {
    if (_ptr >= _end) {
      return {};
    }

    if constexpr (num::float_<T>) {
      return this->parse_flt<T>();
    } else if constexpr (num::uint_<T>) {
      return this->parse_uint<T>();
    } else if constexpr (num::sint_<T>) {
      return this->parse_sint<T>();
    }
  }

 private:
  // 0: end of input
  // *: the char
  [[gnu::always_inline]] auto pop() -> char {
    if (_ptr >= _end) return 0;
    return *_ptr++;
  }

  // 0: end of input, or not matched
  // *: the char
  [[gnu::always_inline]] auto pop_if(auto... c) -> char {
    if (_ptr >= _end) return 0;
    if (((*_ptr != c) && ...)) return 0;
    return *_ptr++;
  }

  //  [0+] : int number
  //  [-1] : failed
  [[gnu::always_inline]] auto pop_digit() -> int {
    if (_ptr >= _end) {
      return -1;
    }

    const auto n = *_ptr - '0';
    if (n < 0 || n > 9) {
      return -1;
    }

    _ptr += 1;
    return n;
  }

  // [0+] : int number
  // [-1] : failed
  [[gnu::always_inline]] auto pop_xdigit(i32 radix) -> int {
    if (_ptr >= _end) {
      return -1;
    }

    const auto n = *_ptr - '0';
    if (0 <= n && n < radix) {
      _ptr += 1;
      return n;
    }

    if (radix < 16) {
      return -1;
    }

    const auto x = 10 + ((*_ptr | 32) - 'a');
    if (0 <= x && x < radix) {
      _ptr += 1;
      return x;
    }

    return -1;
  }

  // +1:  positive
  // -1:  negative
  auto extract_sign() -> int {
    const auto ch = this->pop_if('+', '-');
    return ch == '-' ? -1 : 1;
  }

  auto extract_radix() -> u16 {
    const auto cnt = _end - _ptr;
    // 1. single value will be decimal
    // 2. not with leading '0', will be decimal
    if (cnt <= 1 || _ptr[0] != '0') {
      return 10;
    }

    switch (_ptr[1]) {
      case 'b':
      case 'B': _ptr += 2; return 2;
      case 'x':
      case 'X': _ptr += 2; return 16;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7': _ptr += 1; return 8;
      default:  return 10;
    }
  }

  auto extract_dec() -> u64 {
    auto val = u64{0};
    while (true) {
      const auto n = this->pop_digit();
      if (n == -1) break;
      val = 10 * val + static_cast<u32>(n);
    }
    return val;
  }

  auto extract_bin(u32 radix) -> u64 {
    auto val = u64{0};
    while (true) {
      const auto n = this->pop_xdigit(radix);
      if (n == -1) break;
      val = radix * val + n;
    };
    return val;
  }

  auto extract_flt() -> f64 {
    if (!this->pop_if('.')) {
      return 0.0;
    }

    i64 uval = 0;
    i64 base = 1;
    while (true) {
      const auto n = this->pop_digit();
      if (n == -1) break;
      base *= 10;
      uval = 10 * uval + n;
    }
    return static_cast<f64>(uval) / static_cast<f64>(base);
  }

  auto extract_exp() -> f64 {
    if (_end - _ptr < 2) {
      return 1e0;
    }
    if (!this->pop_if('e', 'E')) {
      return 1e0;
    }

    const auto sign = this->extract_sign();
    const auto scnt = static_cast<i32>(this->extract_dec());
    return fast_exp10(sign * scnt);
  }
};

template <class T>
auto FromStr<T>::from_str(Str s) -> Option<T> {
  auto imp = NumParser{s};
  auto res = imp.parse_num<T>();
  if (!imp.is_empty()) {
    return {};
  }
  return res;
}

template struct FromStr<signed char>;
template struct FromStr<short>;
template struct FromStr<int>;
template struct FromStr<long>;
template struct FromStr<long long>;

template struct FromStr<unsigned char>;
template struct FromStr<unsigned short>;
template struct FromStr<unsigned int>;
template struct FromStr<unsigned long>;
template struct FromStr<unsigned long long>;

template struct FromStr<float>;
template struct FromStr<double>;
}  // namespace sfc::str
