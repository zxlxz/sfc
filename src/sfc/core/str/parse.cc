#include "sfc/core/str.h"

namespace sfc::str {

static auto fast_exp10(i32 val) -> f64 {
  if (val == 0) return 1.0;

  auto res = 1.0;
  if (val > 0) {
    for (auto i = 0; i < val; ++i) {
      res *= 10.0;
    }
  } else {
    for (auto i = 0; i > val; --i) {
      res /= 10.0;
    }
  }
  return res;
}

struct NumParser {
  const char* _ptr;
  const char* _end;

 public:
  auto is_empty() const -> bool {
    return _ptr == _end;
  }

  template <class T>
  auto parse_int() -> Option<T> {
    static constexpr auto MAX_VAL = static_cast<u64>(num::max_value<T>());

    const auto is_neg = this->extract_sign();
    if constexpr (trait::uint_<T>) {
      if (is_neg) return {};
    }

    const auto radix = this->extract_radix();
    const auto uval = this->extract_bin(radix);
    if (uval > MAX_VAL) return {};

    if constexpr (trait::uint_<T>) {
      return static_cast<T>(uval);
    } else {
      if (is_neg && uval && (uval - 1 > MAX_VAL)) return {};
      const auto sval = static_cast<T>(uval);
      return is_neg ? -sval : sval;
    }
  }

  template <class T>
  auto parse_flt() -> Option<T> {
    const auto is_neg = this->extract_sign();
    const auto int_part = this->extract_int();
    const auto flt_part = this->extract_flt();
    const auto exp_part = this->extract_exp();

    const auto uval = (static_cast<f64>(int_part) + flt_part) * exp_part;
    const auto sval = static_cast<T>(uval);
    return is_neg ? -sval : +sval;
  }

  template <class T>
  auto parse_num() -> Option<T> {
    if constexpr (trait::flt_<T>) {
      return this->parse_flt<T>();
    } else if constexpr (trait::int_<T>) {
      return this->parse_int<T>();
    } else {
      return {};
    }
  }

 private:
  auto extract_sign() -> bool {
    if (_ptr == _end) {
      return false;
    }
    switch (*_ptr) {
      default:  return false;
      case '+': ++_ptr; return false;
      case '-': ++_ptr; return true;
    }
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

  auto extract_bin(u32 radix) -> u64 {
    auto val = u64{0};
    for (; _ptr != _end; ++_ptr) {
      const auto c = *_ptr;
      const auto n = static_cast<u32>(c <= '9' ? c - '0' : (c | 32) - ('a' - 10));
      if (n >= radix) {
        break;
      }
      val = radix * val + n;
    }
    return val;
  }

  auto extract_int() -> u64 {
    auto val = u64{0};
    for (; _ptr != _end; ++_ptr) {
      const auto c = *_ptr;
      const auto n = static_cast<u32>(c - '0');
      if (n >= 10) {
        break;
      }
      val = 10 * val + n;
    }
    return val;
  }

  auto extract_flt() -> f64 {
    if (_ptr == _end || *_ptr != '.') {
      return 0.0;
    }
    ++_ptr;

    i64 uval = 0;
    i64 base = 1;
    for (; _ptr != _end; ++_ptr) {
      const auto c = *_ptr;
      const auto n = static_cast<u8>(c - '0');
      if (n >= 10) {
        break;
      }
      base *= 10;
      uval = 10 * uval + n;
    }
    return static_cast<f64>(uval) / static_cast<f64>(base);
  }

  auto extract_exp() -> f64 {
    if (_end - _ptr < 2) {
      return 1e0;
    }
    if (auto c = *_ptr; c != 'e' && c != 'E') {
      return 1e0;
    }
    ++_ptr;

    const auto sign = this->extract_sign() ? -1 : 1;
    const auto scnt = static_cast<i32>(this->extract_int());
    return fast_exp10(sign * scnt);
  }
};

template <class T>
auto FromStr<T>::from_str(Str s) -> Option<T> {
  if (s.is_empty()) {
    return {};
  }

  auto imp = NumParser{s._ptr, s._ptr + s._len};
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
