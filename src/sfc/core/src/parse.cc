#include "sfc/core/str.h"

namespace sfc::str {

static auto fast_exp10(u64 n) -> f64 {
  static constexpr f64 TBL[] = {1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15};

  if (n == 0) {
    return 1.0;
  }

  auto res = 1.0;
  for (; n >= 256; n -= 256) {
    res *= 1e256;
  }
  for (; n >= 16; n -= 16) {
    res *= 1e16;
  }
  res *= TBL[n];

  return res;
}

struct NumParser {
  const char* _ptr;
  const char* _end;

  struct Digit {
    enum Tag {
      Ok,
      Eof,
      Invalid,
    };
    Tag tag;
    u32 val;

   public:
    operator bool() const noexcept {
      return tag == Ok;
    }
  };

 public:
  NumParser(Str s) noexcept : _ptr{s._ptr}, _end{s._ptr + s._len} {}

  auto is_empty() const -> bool {
    return _ptr == _end;
  }

  template <trait::uint_ T>
  auto parse_uint() -> Option<T> {
    static constexpr auto kMaxVal = num::max_value<T>();

    const auto is_neg = this->extract_sign() == '-';
    if (is_neg) return {};

    const auto radix = this->extract_radix();
    const auto uval = radix == 10 ? this->extract_dec() : this->extract_bin(radix);
    if (uval > kMaxVal) return {};

    return static_cast<T>(uval);
  }

  template <trait::sint_ T>
  auto parse_sint() -> Option<T> {
    static constexpr auto kMaxVal = num::cast_unsigned(num::max_value<T>());

    const auto is_neg = this->extract_sign() == '-';
    const auto radix = this->extract_radix();
    const auto uval = radix == 10 ? this->extract_dec() : this->extract_bin(radix);

    if (uval > kMaxVal) {
      if (is_neg && uval == kMaxVal + 1) {
        return num::min_value<T>();
      }
      return {};
    }

    const auto sval = static_cast<T>(uval);
    return is_neg ? -sval : sval;
  }

  template <class T>
  auto parse_flt() -> Option<T> {
    const auto sign = this->extract_sign() == '-' ? -1.0 : 1.0;

    const auto int_part = static_cast<f64>(this->extract_dec());
    const auto flt_part = this->extract_flt();
    const auto exp_part = this->extract_exp();

    const auto flt_val = sign * (int_part + flt_part) * exp_part;
    return static_cast<T>(flt_val);
  }

  template <class T>
  auto parse_num() -> Option<T> {
    if (_ptr >= _end) {
      return {};
    }

    if constexpr (trait::float_<T>) {
      return this->parse_flt<T>();
    } else if constexpr (trait::uint_<T>) {
      return this->parse_uint<T>();
    } else if constexpr (trait::sint_<T>) {
      return this->parse_sint<T>();
    }
  }

 private:
  // 0: end of input
  // *: the char
  auto pop() -> char {
    if (_ptr >= _end) return 0;
    return *_ptr++;
  }

  // 0: end of input, or not matched
  // *: the char
  auto pop_if(auto... c) -> char {
    if (_ptr >= _end) return 0;
    if (((*_ptr != c) && ...)) return 0;
    return *_ptr++;
  }

  //  [0+] : int number
  //  [-1] : failed
  auto pop_digit() -> Digit {
    if (_ptr >= _end) {
      return Digit{Digit::Eof, 0};
    }

    const auto n = num::cast_unsigned(*_ptr - '0');
    if (n > 9) {
      return Digit{Digit::Invalid, 0};
    }

    _ptr += 1;
    return Digit{Digit::Ok, n};
  }

  // [0+] : int number
  // [-1] : failed
  auto pop_xdigit(u32 radix) -> Digit {
    if (_ptr >= _end) {
      return Digit{Digit::Eof, 0};
    }

    const auto c = *_ptr | 32;  // to lower
    const auto n = num::cast_unsigned(c - '0');
    if (n < radix) {
      _ptr += 1;
      return Digit{Digit::Ok, n};
    }

    if (radix != 16 || (c < 'a' || c > 'f')) {
      return Digit{Digit::Invalid, 0};
    }

    const auto x = 10 + num::cast_unsigned(c - 'a');
    _ptr += 1;
    return Digit{Digit::Ok, x};
  }

  // +1:  positive
  // -1:  negative
  auto extract_sign() -> char {
    const auto ch = this->pop_if('+', '-');
    return ch;
  }

  auto extract_radix() -> u32 {
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
    auto res = u64{0};
    while (auto digit = this->pop_digit()) {
      res = 10 * res + digit.val;
    }
    return res;
  }

  auto extract_bin(u32 radix) -> u64 {
    auto res = u64{0};
    while (auto digit = this->pop_xdigit(radix)) {
      res = radix * res + digit.val;
    }
    return res;
  }

  auto extract_flt() -> f64 {
    if (!this->pop_if('.')) {
      return 0.0;
    }

    auto uval = 0.0;
    auto base = 0.1;
    while (auto digit = this->pop_digit()) {
      uval += base * digit.val;
      base *= 0.1;
    }
    return uval;
  }

  auto extract_exp() -> f64 {
    if (_end - _ptr < 2) {
      return 1e0;
    }

    if (!this->pop_if('e', 'E')) {
      return 1e0;
    }

    const auto sign = this->extract_sign();
    const auto scnt = this->extract_dec();
    const auto uexp = fast_exp10(scnt);
    return sign == '-' ? 1.0 / uexp : uexp;
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
