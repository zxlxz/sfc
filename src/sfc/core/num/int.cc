#include "sfc/core/num.h"
#include "sfc/core/str.h"

namespace sfc::num {

namespace {
struct IntFmter {
  char* const _buf;
  char* const _end;
  char* _ptr = _end;  // write backwards

 public:
  auto as_str() const -> Str {
    return Str{_ptr, static_cast<usize>(_end - _ptr)};
  }

  auto push(char c) noexcept -> usize {
    if (_ptr == _buf) {
      return 0;
    }
    *--_ptr = c;
    return 1;
  }

  auto write_dec(auto val) noexcept -> usize {
    static const char DIGITS[] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";

    auto nret = 0U;

    // write digits
    auto uval = val > 0 ? val : 0 - val;
    for (; uval >= 100; uval /= 100) {
      const auto n = uval % 100 * 2;
      nret += this->push(DIGITS[n + 1]);
      nret += this->push(DIGITS[n + 0]);
    }
    if (uval >= 10) {
      const auto n = uval % 100 * 2;
      nret += this->push(DIGITS[n + 1]);
      nret += this->push(DIGITS[n + 0]);
    } else {
      nret += this->push(static_cast<char>(uval + '0'));
    }

    // write sign
    if constexpr (trait::sint_<decltype(val)>) {
      if (val < 0) {
        nret += this->push('-');
      }
    }

    return nret;
  }

  template <u32 RADIX>
  auto write_bin(auto val, bool upcase = false) noexcept -> usize {
    static constexpr auto UPCASE = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static constexpr auto LOWCASE = "0123456789abcdefghijklmnopqrstuvwxyz";
    static constexpr auto MASK = RADIX - 1;
    static_assert((RADIX & (RADIX - 1)) == 0, "radix must be power of 2");

    auto nret = 0U;

    // write digits
    const auto DIGITS = upcase ? UPCASE : LOWCASE;
    auto uval = val > 0 ? val : 0 - val;
    if (uval == 0) {
      nret += this->push('0');
    } else {
      for (; uval != 0; uval /= RADIX) {
        nret += this->push(DIGITS[uval & MASK]);
      }
    }

    // write sign
    if constexpr (trait::sint_<decltype(val)>) {
      if (val < 0) {
        nret += this->push('-');
      }
    }

    return nret;
  }

  auto write_ptr(const void* val, bool upcase = false) noexcept -> usize {
    static const auto MIN_LEN = 12L;

    const auto uval = reinterpret_cast<usize>(val);
    auto nret = this->write_bin<16>(uval, upcase);

    // pad '0', to at least MIN_LEN characters
    for (; nret < MIN_LEN; ++nret) {
      nret += this->push('0');
    }

    return nret;
  }
};

struct IntParser {
  const char* _ptr;
  const char* _end;

 public:
  template <trait::uint_ T>
  auto read(T& dst, u32 radix) noexcept -> bool {
    static constinit auto MAX = max_value<T>();

    // extract sign
    const auto is_neg = this->extract_sign();
    if (is_neg) {
      return false;
    }

    // extract radix
    // if radix == 0, extract_radix to determine it
    if (radix == 0) {
      radix = this->extract_radix();
    }

    // extract digits
    if (_ptr == _end) {
      return false;
    }
    const auto uval = this->extract_digits(radix, MAX);
    dst = static_cast<T>(uval);

    return _ptr == _end;
  }

  template <trait::sint_ T>
  auto read(T& dst, u32 radix) noexcept -> bool {
    static constinit auto MAX = static_cast<u64>(max_value<T>()) + 1;

    // extract sign
    const auto is_neg = this->extract_sign();

    // extract radix
    if (radix == 0) {
      radix = this->extract_radix();
    }

    // extract digits
    if (_ptr == _end) {
      return false;
    }
    const auto uval = this->extract_digits(radix, MAX);
    if (uval == MAX && !is_neg) {  // check overflow
      return false;
    }
    dst = is_neg ? -static_cast<T>(uval) : static_cast<T>(uval);

    return _ptr == _end;
  }

 private:
  auto extract_sign() noexcept -> bool {
    if (_ptr == _end) {
      return false;
    }

    switch (*_ptr) {
      case '+': ++_ptr; return false;
      case '-': ++_ptr; return true;
      default:  return false;
    }
  }

  auto extract_radix() noexcept -> u32 {
    const auto len = _end - _ptr;

    // 1. single value will be decimal
    // 2. not with leading '0', will be decimal
    if (len <= 1 || _ptr[0] != '0') {
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

  auto extract_digits(u32 radix, u64 max_val) -> u64 {
    const auto A = max_val / radix;
    const auto B = max_val % radix;

    auto val = 0uz;
    for (; _ptr != _end; ++_ptr) {
      const auto c = *_ptr;
      const auto n = static_cast<u32>(c <= '9' ? c - '0' : (c | 32) - ('a' - 10));
      if (n >= radix) {
        break;
      }
      if (val > A || (val == A && n > B)) {
        break;
      }
      val = radix * val + n;
    }
    return val;
  }
};
}  // namespace

auto to_str(Slice<char> buf, const void* val, char type) noexcept -> Str {
  auto imp = IntFmter{buf._ptr, buf._ptr + buf._len};
  imp.write_ptr(val, type);
  return imp.as_str();
}

auto to_str(Slice<char> buf, trait::int_ auto val, char type) noexcept -> Str {
  auto imp = IntFmter{buf._ptr, buf._ptr + buf._len};
  switch (type) {
    default:  imp.write_dec(val + 0); break;
    case 'B': imp.write_bin<2>(val + 0); break;
    case 'b': imp.write_bin<2>(val + 0); break;
    case 'O': imp.write_bin<8>(val + 0); break;
    case 'o': imp.write_bin<8>(val + 0); break;
    case 'X': imp.write_bin<16>(val + 0, true); break;
    case 'x': imp.write_bin<16>(val + 0, false); break;
    case 'P': imp.write_bin<16>(val + 0, true); break;
    case 'p': imp.write_bin<16>(val + 0, false); break;
  }
  return imp.as_str();
}

template <trait::int_ T>
auto from_str(Str buf, u32 radix) noexcept -> Option<T> {
  auto imp = IntParser{buf._ptr, buf._ptr + buf._len};
  auto val = T{};

  if (!imp.read(val, radix)) {
    return {};
  }
  return Option{val};
}

template auto to_str(Slice<char>, signed char, char) noexcept -> Str;
template auto to_str(Slice<char>, short, char) noexcept -> Str;
template auto to_str(Slice<char>, int, char) noexcept -> Str;
template auto to_str(Slice<char>, long, char) noexcept -> Str;
template auto to_str(Slice<char>, long long, char) noexcept -> Str;

template auto to_str(Slice<char>, unsigned char, char) noexcept -> Str;
template auto to_str(Slice<char>, unsigned short, char) noexcept -> Str;
template auto to_str(Slice<char>, unsigned int, char) noexcept -> Str;
template auto to_str(Slice<char>, unsigned long, char) noexcept -> Str;
template auto to_str(Slice<char>, unsigned long long, char) noexcept -> Str;

template auto from_str(Str, u32 radix) noexcept -> Option<signed char>;
template auto from_str(Str, u32 radix) noexcept -> Option<short>;
template auto from_str(Str, u32 radix) noexcept -> Option<int>;
template auto from_str(Str, u32 radix) noexcept -> Option<long>;
template auto from_str(Str, u32 radix) noexcept -> Option<long long>;

template auto from_str(Str, u32 radix) noexcept -> Option<unsigned char>;
template auto from_str(Str, u32 radix) noexcept -> Option<unsigned short>;
template auto from_str(Str, u32 radix) noexcept -> Option<unsigned int>;
template auto from_str(Str, u32 radix) noexcept -> Option<unsigned long>;
template auto from_str(Str, u32 radix) noexcept -> Option<unsigned long long>;
}  // namespace sfc::num
