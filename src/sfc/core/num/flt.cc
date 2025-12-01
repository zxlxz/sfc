#include "sfc/core/num.h"
#include "sfc/core/str.h"

namespace sfc::num {

struct ParseFlt {
  const char* _ptr;
  const char* _end;

 public:
  template <class T>
  auto parse() noexcept -> Option<T> {
    const auto is_neg = this->extract_sign();
    const auto int_part = this->extract_int_part();
    const auto flt_part = this->extract_flt_part();
    if (_ptr != _end) {
      return {};
    }

    const auto val = static_cast<T>(int_part + flt_part);
    return is_neg ? -val : val;
  }

 private:
  auto extract_sign() noexcept -> bool {
    if (_ptr == _end) {
      return false;
    }

    const auto c = *_ptr;
    if (c == '+' || c == '-') {
      ++_ptr;
    }
    return c == '-';
  }

  auto extract_int_part() noexcept -> f64 {
    i64 res = 0;
    for (; _ptr != _end; ++_ptr) {
      const auto c = *_ptr;
      const auto n = static_cast<u8>(c - '0');
      if (n >= 10) {
        break;
      }
      res = 10 * res + n;
    }
    return static_cast<f64>(res);
  }

  auto extract_flt_part() noexcept -> f64 {
    if (_ptr == _end || *_ptr != '.') {
      return 0;
    }
    ++_ptr;

    i64 val = 0;
    i64 exp = 1;
    for (; _ptr != _end; ++_ptr) {
      const auto c = *_ptr;
      const auto n = static_cast<u8>(c - '0');
      if (n >= 10) {
        break;
      }

      exp *= 10;
      val = 10 * val + n;
    }
    return static_cast<f64>(val) / exp;
  }
};

auto flt_eq_ulp(f64 a, f64 b, u32 ulp) noexcept -> bool {
  if (__builtin_isnan(a) || __builtin_isnan(b)) {
    return false;
  }

  if (__builtin_isinf(a) || __builtin_isinf(b)) {
    return a == b;
  }

  const auto ia = __builtin_bit_cast(i64, a);
  const auto ib = __builtin_bit_cast(i64, b);
  if ((ia ^ ib) < 0) {
    return a == b;
  }

  const auto diff = static_cast<u32>(ia > ib ? ia - ib : ib - ia);
  return diff <= ulp;
}

}  // namespace sfc::num

namespace sfc::str {

template <class T>
auto FromStr<T>::from_str(Str s) -> Option<T> {
  auto imp = num::ParseFlt{s._ptr, s._ptr + s._len};
  return imp.parse<T>();
}

template struct FromStr<f32>;
template struct FromStr<f64>;

}  // namespace sfc::str
