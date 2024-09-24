#include "sfc/core/num.h"
#include "sfc/core/str.h"

namespace sfc::str {

struct Str2Flt {
  slice::Slice<const char> _s;

 public:
  template <class T>
  auto parse_flt() -> option::Option<T> {
    const auto sign = T{this->extract_sign()};
    const auto nan_inf = this->extract_nan_inf(sign);
    if (nan_inf) {
      return nan_inf;
    }

    const auto int_part = this->extract_int_part();
    const auto flt_part = this->extract_flt_part();
    if (_s._len != 0) {
      return {};
    }

    const auto res = static_cast<T>(sign * (int_part + flt_part));
    return res;
  }

 private:
  [[sfc_inline]] void pop() {
    _s._ptr += 1;
    _s._len -= 1;
  }

  auto extract_sign() -> f32 {
    const auto c = _s._len ? _s._ptr[0] : char(0);
    if (c == '+' || c == '-') {
      this->pop();
    }
    return c == '-' ? -1.0f : 1.0f;
  }

  template <class T>
  auto extract_nan_inf(T sign) -> option::Option<T> {
    if (_s._len != 3) {
      return {};
    };
    const auto p = _s._ptr;
    if ((p[0] | 32) == 'n' && (p[1] | 32) == 'a' && (p[2] | 32) == 'n') {
      return T{__builtin_nanf("")};
    }
    if ((p[0] | 32) == 'i' && (p[1] | 32) == 'n' && (p[2] | 32) == 'f') {
      return T{sign * __builtin_inff()};
    }
    return {};
  }

  auto extract_int_part() -> f64 {
    f64 res = 0.0;
    for (; _s._len != 0; this->pop()) {
      const auto c = _s._ptr[0];
      const auto n = static_cast<u8>(c - '0');
      if (n >= 10) break;
      res = 10 * res + n;
    }
    return res;
  }

  auto extract_flt_part() -> f64 {
    if (_s._len == 0 || _s._ptr[0] != '.') {
      return 0;
    }
    this->pop();

    f64 res = 0.0;
    f64 exp = 1.0;
    for (; _s._len != 0; this->pop()) {
      const auto c = _s._ptr[0];
      const auto n = static_cast<u8>(c - '0');
      if (n >= 10) break;

      exp /= 10.0;
      res += n * exp;
    }
    return res;
  }
};

template <class T>
auto Str::parse() const -> option::Option<T> {
  auto imp = Str2Flt{{_ptr, _len}};
  return imp.parse_flt<T>();
}

template auto Str::parse<f32>() const -> option::Option<f32>;
template auto Str::parse<f64>() const -> option::Option<f64>;

}  // namespace sfc::str
