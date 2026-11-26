#include "sfc/core/str.h"
#include "sfc/core/fmt.h"
#include "sfc/core/cmp.h"

namespace sfc::str {

namespace {

auto fast_exp10(i32 p) -> f64 {
  static constexpr f64 TBL[] = {1e0, 1e1, 1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8, 1e9, 1e10, 1e11, 1e12, 1e13, 1e14, 1e15};

  auto n = num::unsigned_abs(p);
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

  if (n < 16) {
#ifndef __clang_analyzer__  // make clang-analyzer happy
    res *= TBL[n];
#endif
  }

  if (p < 0) {
    res = 1.0 / res;
  }
  return res;
}
}  // namespace

auto Str::trim_start() const noexcept -> Str {
  const auto is_space = [](char c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); };
  return this->trim_start_matches(is_space);
}

auto Str::trim_end() const noexcept -> Str {
  const auto is_space = [](char c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); };
  return this->trim_end_matches(is_space);
}

auto Str::trim() const noexcept -> Str {
  const auto is_space = [](char c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); };
  return this->trim_matches(is_space);
}

void Str::fmt(fmt::Formatter& f) const {
  const auto type = f.type();
  if (type == 's' || type == '?') {
    f.write_char('"');
    f.write_str(*this);
    f.write_char('"');
  } else {
    f.pad(*this);
  }
}

class NumParser {
  const char* _ptr;
  const char* _end;

 public:
  NumParser(Str s) : _ptr{s._ptr}, _end{s._ptr + s._len} {}

 public:
  template <trait::uint_ T>
  auto parse_num() -> Option<T> {
    static constexpr u64 kMaxVal = num::Int<T>::MAX;

    if (this->is_empty()) {
      return {};
    }

    const auto sign = this->next_sign();
    if (sign == -1) return {};

    const auto radix = this->next_radix();
    const auto [uval, cnt] = this->next_uint<u64>(radix);
    if (cnt == 0) return {};
    if (uval > kMaxVal) return {};
    if (!this->is_empty()) return {};
    return T(uval);
  }

  template <trait::sint_ T>
  auto parse_num() -> Option<T> {
    static constexpr u64 kMaxVal = num::Int<T>::MAX;

    if (this->is_empty()) {
      return {};
    }

    const auto sign = this->next_sign();
    const auto radix = this->next_radix();
    const auto [uval, cnt] = this->next_uint<u64>(radix);
    if (cnt == 0 || !this->is_empty()) {
      return {};
    }

    if (sign > 0) {
      if (uval > kMaxVal) return {};
      return T(uval);
    }

    if (sign < 0) {
      if (uval > kMaxVal + 1) return {};
      return T(0 - i64(uval));
    }

    return {};
  }

  template <trait::flt_ T>
  auto parse_num() -> Option<T> {
    if (this->is_empty()) {
      return {};
    }

    const auto sign = this->next_sign();
    const auto [int_val, int_cnt] = this->next_uint<u64>();
    if (int_cnt == 0) return {};  // don't support float like ".123" or "-.456"

    auto flt_val = 0.0;
    auto exp_val = 1.0;
    if (this->next_point() != 0) {
      const auto [tmp_val, digits_cnt] = this->next_uint<u64>();
      flt_val = f64(tmp_val) * fast_exp10(-i32(digits_cnt));
    }
    if (this->next_exp() != 0) {
      const auto exp_sign = this->next_sign();
      const auto [tmp_val, digits_cnt] = this->next_uint<u16>();
      if (digits_cnt == 0) return {};
      exp_val = fast_exp10(exp_sign * int(tmp_val));
    }

    if (!this->is_empty()) return {};

    const auto base_val = f64(int_val) + flt_val;
    const auto val = sign * base_val * exp_val;
    return T(val);
  }

 private:
  auto is_empty() const -> bool {
    return _ptr >= _end;
  }

  auto pop_matches(auto... c) -> char {
    if (_ptr >= _end) return 0;
    if (((*_ptr != c) && ...)) return 0;
    return *_ptr++;
  }

  auto next_point() -> char {
    const auto c = this->pop_matches('.');
    return c;
  }

  auto next_sign() -> int {
    const auto c = this->pop_matches('+', '-');
    return c == '-' ? -1 : 1;
  }

  auto next_exp() -> char {
    const auto c = this->pop_matches('e', 'E');
    return c;
  }

  auto next_radix() -> u16 {
    if (_ptr + 1 >= _end) {
      return 10;
    }

    const auto h = _ptr[0];
    if (h != '0') {
      return 10;
    }

    const auto r = _ptr[1];
    switch (r) {
      default:  return 10;
      case 'x':
      case 'X': _ptr += 2; return 16;
      case 'b':
      case 'B': _ptr += 2; return 2;
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7': _ptr += 1; return 8;
    }

    return 10;
  }

  template <trait::uint_ T>
  auto next_uint(u16 radix = 10) -> Tuple<T, u32> {
    static constexpr auto kU64Max = num::Int<u64>::MAX;

    if (_ptr >= _end) {
      return {0, 0};
    }

    auto n = 0U;
    auto s = u64{0};
    for (; _ptr + n < _end; ++n) {
      const auto c = _ptr[n] | 32;  // to lower
      const auto d = u16(c - '0');
      const auto x = u16(c - 'a' + 10);
      const auto t = d <= 9 ? d : x;
      if (t >= radix) {
        break;
      }
      if (s > (kU64Max - t) / radix) {
        break;
      }
      s = s * radix + u16(t);
    }

    if (s > num::Int<T>::MAX) {
      return {0, 0};
    }

    _ptr += n;
    return {T(s), n};
  }
};

template <class T>
auto FromStr<T>::from_str(Str s) -> Option<T> {
  auto parser = NumParser{s};
  return parser.parse_num<T>();
}

template struct FromStr<unsigned char>;
template struct FromStr<unsigned short>;
template struct FromStr<unsigned int>;
template struct FromStr<unsigned long>;
template struct FromStr<unsigned long long>;

template struct FromStr<signed char>;
template struct FromStr<signed short>;
template struct FromStr<signed int>;
template struct FromStr<signed long>;
template struct FromStr<signed long long>;

template struct FromStr<float>;
template struct FromStr<double>;


}  // namespace sfc::str
