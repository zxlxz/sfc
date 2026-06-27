#include "sfc/core/str.h"
#include "sfc/core/hash.h"

namespace sfc::str {

struct Parser {
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
  Parser(Str s) noexcept : _ptr{s._ptr}, _end{s._ptr + s._len} {}

  auto is_empty() const -> bool {
    return _ptr == _end;
  }

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

  template <trait::uint_ T>
  auto parse_uint() -> Option<T> {
    static constexpr auto kMaxVal = num::Int<T>::MAX;

    if (auto sign = this->extract_sign(); sign == '-') {
      return {};
    }

    const auto radix = this->extract_radix();
    const auto uval = radix == 10 ? this->extract_dec() : this->extract_bin(radix);
    if (uval > kMaxVal) {
      return {};
    }

    return static_cast<T>(uval);
  }

  template <trait::sint_ T>
  auto parse_sint() -> Option<T> {
    static constexpr auto kMaxVal = num::Int<T>::MAX;
    static constexpr auto kMinVal = num::Int<T>::MIN;

    const auto is_neg = this->extract_sign() == '-';
    const auto radix = this->extract_radix();
    const auto uval = radix == 10 ? this->extract_dec() : this->extract_bin(radix);
    const auto ival = num::cast_signed(is_neg ? 0 - uval : uval);

    if (ival > kMaxVal || ival < kMinVal) {
      return {};
    }

    return static_cast<T>(ival);
  }

  template <class T>
  auto parse_flt() -> Option<T> {
    const auto sign = this->extract_sign() == '-' ? -1.0 : 1.0;

    const auto int_part = f64(this->extract_dec());
    const auto flt_part = this->extract_flt();
    const auto exp_part = this->extract_exp();

    const auto flt_val = sign * (int_part + flt_part) * exp_part;
    return T(flt_val);
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

    auto val = u64{0};
    auto cnt = 0U;
    while (auto digit = this->pop_digit()) {
      val = 10 * val + digit.val;
      cnt += 1;
    }

    const auto exp_val = fast_exp10(cnt);
    const auto flt_val = f64(val) / exp_val;
    return flt_val;
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
  auto imp = Parser{s};
  auto res = imp.parse_num<T>();
  if (!imp.is_empty()) {
    return {};
  }
  return res;
}

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

auto Str::hash() const noexcept -> usize {
  auto hasher = hash::Hasher{};
  hasher.write(this->as_bytes());
  return hasher.finish();
}

auto CharSearcher::next() -> SearchStep {
  if (_finger >= _haystack._len) {
    return {SearchStep::Done, 0, 0};
  }

  const auto ch = _haystack[_finger++];
  if (ch == _needle) {
    return {SearchStep::Match, _finger - 1, _finger};
  } else {
    return {SearchStep::Reject, _finger - 1, _finger};
  }
}

auto CharSearcher::next_back() -> SearchStep {
  if (_finger_back == 0) {
    return {SearchStep::Done, 0, 0};
  }

  const auto ch = _haystack[_finger_back - 1];
  if (ch == _needle) {
    _finger_back -= 1;
    return {SearchStep::Match, _finger_back, _finger_back + 1};
  } else {
    _finger_back -= 1;
    return {SearchStep::Reject, _finger_back, _finger_back + 1};
  }
}

auto StrSearcher::match() const -> bool {
  if (_needle._len == 0) return true;
  if (_finger + _needle._len > _haystack._len) return false;

  const auto p = _haystack._ptr + _finger;
  return __builtin_memcmp(p, _needle._ptr, _needle._len) == 0;
}

auto StrSearcher::match_back() const -> bool {
  if (_needle._len == 0) return true;
  if (_finger_back < _needle._len) return false;

  const auto p = _haystack._ptr + _finger_back - _needle._len;
  return __builtin_memcmp(p, _needle._ptr, _needle._len) == 0;
}

auto StrSearcher::next() -> SearchStep {
  if (_finger >= _haystack._len) {
    return {SearchStep::Done, 0, 0};
  }

  const auto old_finger = _finger;
  if (this->match()) {
    _finger += _needle._len;
    return {SearchStep::Match, old_finger, _finger};
  } else {
    if (_finger + _needle._len < _haystack._len) {
      _finger += 1;
    } else {
      _finger = _haystack._len;
    }
    return {SearchStep::Reject, old_finger, _finger};
  }
}

auto StrSearcher::next_back() -> SearchStep {
  if (_finger_back == 0) {
    return {SearchStep::Done, 0, 0};
  }

  const auto old_finger_back = _finger_back;
  if (this->match_back()) {
    _finger_back -= _needle._len;
    return {SearchStep::Match, _finger_back, old_finger_back};
  } else {
    if (_finger_back >= _needle._len) {
      _finger_back -= _needle._len;
    } else {
      _finger_back = 0;
    }
    return {SearchStep::Reject, _finger_back, old_finger_back};
  }
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
