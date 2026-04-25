#include "sfc/core/str.h"

namespace sfc::str {

static constexpr auto kInvalidChar = char32_t{0xFFFD};

static auto utf8_codelen(u8 h) -> u32 {
  if ((h & 0x80) == 0) {
    return 1;
  } else if ((h & 0xE0) == 0xC0) {
    return 2;
  } else if ((h & 0xF0) == 0xE0) {
    return 3;
  } else if ((h & 0xF8) == 0xF0) {
    return 4;
  }
  return 0;
}

static auto utf8_codepoint_prev(const char* begin, const char* end) -> const char* {
  static constexpr auto kMaxUtf8CodeLen = 4U;

  if (begin + kMaxUtf8CodeLen > end) {
    begin = end - kMaxUtf8CodeLen;
  }

  auto p = end - 1;
  while (p > begin && (p[0] & 0xC0) == 0x80) {
    p--;
  }

  return p;
}

static auto utf8_decode(const char* p, u32 n) -> char32_t {
  switch (n) {
    case 1: {
      const auto a = p[0];
      return static_cast<char32_t>(a);
    }
    case 2: {
      const auto a = p[0] & 0x1F;
      const auto b = p[1] & 0x3F;
      return static_cast<char32_t>((a << 6) | b);
    }
    case 3: {
      const auto a = p[0] & 0x0F;
      const auto b = p[1] & 0x3F;
      const auto c = p[2] & 0x3F;
      return static_cast<char32_t>((a << 12) | (b << 6) | c);
    }
    case 4: {
      const auto a = p[0] & 0x07;
      const auto b = p[1] & 0x3F;
      const auto c = p[2] & 0x3F;
      const auto d = p[3] & 0x3F;
      return static_cast<char32_t>((a << 18) | (b << 12) | (c << 6) | d);
    }
  }
  return kInvalidChar;
}

auto Chars::next() noexcept -> Option<char32_t> {
  if (_ptr >= _end) {
    return {};
  }

  const auto cnt = utf8_codelen(*_ptr);
  // invalid utf8 code-point
  if (cnt == 0) {
    _ptr += 1;
    return kInvalidChar;
  }

  // not enough bytes
  if (_ptr + cnt > _end) {
    _ptr = _end;
    return kInvalidChar;
  }

  const auto ch = utf8_decode(_ptr, cnt);
  _ptr += cnt;

  return ch;
}

auto Chars::next_back() noexcept -> Option<char32_t> {
  if (_ptr >= _end) {
    return {};
  }

  const auto p = str::utf8_codepoint_prev(_ptr, _end);

  // invalid utf8, cannot find code-point
  if (p == nullptr) {
    _end -= 1;
    return kInvalidChar;
  }

  const auto n = str::utf8_codelen(*p);
  if (n == 0) {
    _end -= 1;
    return kInvalidChar;
  }

  // not enough bytes
  if (p + n > _end) {
    _ptr = _end;
    return kInvalidChar;
  }

  const auto ch = str::utf8_decode(p, n);
  _ptr = p + n;
  return ch;
}

struct NumParser {
  const char* _ptr;
  const char* _end;

 public:
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

  auto is_empty() const -> bool {
    return _ptr == _end;
  }

  template <class T>
  auto parse_uint() -> Option<T> {
    static constexpr auto MAX_VAL = static_cast<u64>(num::max_value<T>());

    const auto sign = this->extract_sign();
    if (sign == -1) return {};

    const auto radix = this->extract_radix();
    const auto uval = radix == 10 ? this->extract_int() : this->extract_bin(radix);
    if (uval > MAX_VAL) return {};

    return static_cast<T>(uval);
  }

  template <class T>
  auto parse_sint() -> Option<T> {
    static constexpr auto MAX_VAL = static_cast<u64>(num::max_value<T>());

    const auto sign = this->extract_sign();
    const auto radix = this->extract_radix();
    const auto uval = radix == 10 ? this->extract_int() : this->extract_bin(radix);

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
    const auto int_part = this->extract_int();
    const auto flt_part = this->extract_flt();
    const auto exp_part = this->extract_exp();

    const auto uval = (static_cast<f64>(int_part) + flt_part) * exp_part;
    return static_cast<T>(sign * uval);
  }

  template <class T>
  auto parse_num() -> Option<T> {
    if constexpr (num::float_<T>) {
      return this->parse_flt<T>();
    } else if constexpr (num::uint_<T>) {
      return this->parse_uint<T>();
    } else if constexpr (num::sint_<T>) {
      return this->parse_sint<T>();
    } else {
      static_assert(false, "NumParser::parse_num: unsupported type");
    }
  }

 private:
  [[gnu::always_inline]] auto pop() -> char {
    return _ptr < _end ? *_ptr++ : '\0';
  }

  [[gnu::always_inline]] auto pop_if(auto... c) -> char {
    if (_ptr < _end && ((*_ptr == c) || ...)) {
      return *_ptr++;
    }
    return 0;
  }

  // returns:
  //  [0+] : valid
  //  [-1] : not a valid digit
  [[gnu::always_inline]] auto pop_digit() -> int {
    if (_ptr >= _end) {
      return -1;
    }

    const auto c = *_ptr;
    if (c < '0' || c > '9') {
      return -1;
    }

    _ptr += 1;
    return c - '0';
  }

  // @return
  //  [0+] : valid
  //  [-1] : not a valid digit
  [[gnu::always_inline]] auto pop_xdigit(i32 radix) -> int {
    if (_ptr >= _end) {
      return -1;
    }

    const auto c = *_ptr;
    const auto n = c - '0';
    if (0 <= n && n < radix) {
      _ptr += 1;
      return n;
    }

    if (radix < 16) {
      return -1;
    }

    const auto x = (*_ptr | 32) - 'a' + 10;
    if (0 <= x && x < radix) {
      _ptr += 1;
      return x;
    }

    return -1;
  }

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

  auto extract_int() -> u64 {
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
