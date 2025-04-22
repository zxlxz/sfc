#include "sfc/core/num.h"
#include "sfc/core/str.h"

namespace sfc::str {

namespace {

struct FltStr {
  Str _s;

 public:
  template <class T>
  auto parse_flt() -> Option<T> {
    const auto sign = this->extract_sign();
    const auto int_part = this->extract_int_part();
    const auto flt_part = this->extract_flt_part();
    if (_s._len != 0) {
      return {};
    }

    const auto res = sign * (int_part + flt_part);
    return Option{static_cast<T>(res)};
  }

 private:
  void pop() {
    _s._ptr += 1;
    _s._len -= 1;
  }

  auto extract_sign() -> f32 {
    const auto c = _s._len ? _s._ptr[0] : '\0';
    if (c == '+' || c == '-') {
      this->pop();
    }
    return c == '-' ? -1.0f : 1.0f;
  }

  auto extract_int_part() -> f64 {
    f64 res = 0.0;
    for (; _s._len != 0; this->pop()) {
      const auto c = _s._ptr[0];
      const auto n = static_cast<u8>(c - '0');
      if (n >= 10) {
        break;
      }
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
      if (n >= 10) {
        break;
      }

      exp /= 10.0;
      res += n * exp;
    }
    return res;
  }
};

}  // namespace

template <trait::isFlt T>
struct FromStr<T> {
  static auto from_str(Str s) -> Option<T> {
    auto imp = FltStr{s};
    return imp.parse_flt<T>();
  }
};

template struct FromStr<f32>;
template struct FromStr<f64>;

}  // namespace sfc::str
