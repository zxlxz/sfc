#include "sfc/core/num.h"
#include "sfc/core/str.h"

namespace sfc::num {

struct IntFmter {
  char* _buf;
  char* _ptr;

 public:
  auto write(auto val, char type) noexcept -> Str {
    const auto tail = _ptr;
    const auto uval = val > 0 ? val : 0 - val;

    switch (type) {
      default:  this->write_dec(uval); break;
      case 'B': this->write_bin<2>(uval); break;
      case 'b': this->write_bin<2>(uval); break;
      case 'O': this->write_bin<8>(uval); break;
      case 'o': this->write_bin<8>(uval); break;
      case 'X': this->write_bin<16, 'X'>(uval); break;
      case 'x': this->write_bin<16, 'x'>(uval); break;
      case 'P': this->write_bin<16, 'P'>(uval); break;
      case 'p': this->write_bin<16, 'p'>(uval); break;
    }

    if (val != uval) {
      this->push('-');
    }

    const auto len = static_cast<usize>(tail - _ptr);
    return {_ptr, len};
  }

 private:
  void push(char c) noexcept {
    if (_ptr == _buf) {
      return;
    }
    *--_ptr = c;
  }

  void write_dec(auto val) noexcept {
    static const char DIGITS[] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";

    for (; val >= 100; val /= 100) {
      const auto n = val % 100 * 2;
      this->push(DIGITS[n + 1]);
      this->push(DIGITS[n + 0]);
    }
    if (val >= 10) {
      const auto n = val % 100 * 2;
      this->push(DIGITS[n + 1]);
      this->push(DIGITS[n + 0]);
    } else {
      this->push(static_cast<char>(val + '0'));
    }
  }

  template <u32 RADIX, char TYPE = 0>
  void write_bin(auto val) noexcept {
    const auto MASK = RADIX - 1;
    const auto DIGITS = TYPE < 'a' ? "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ" : "0123456789abcdefghijklmnopqrstuvwxyz";

    if (val == 0) {
      return this->push('0');
    }

    for (; val != 0; val /= RADIX) {
      this->push(DIGITS[val & MASK]);
    }

    if constexpr (TYPE == 'p' || TYPE == 'P') {
      const auto kMinLen = 12U;
      const auto kCurLen = static_cast<usize>(_ptr - _ptr);
      for (auto i = kCurLen; i < kMinLen; ++i) {
        this->push('0');
      }
    }
  }
};

struct IntParser {
  const char* _ptr;
  const char* _end;

 public:
  auto is_empty() const -> bool {
    return _ptr == _end;
  }

  template <trait::uint_ T>
  auto read(T& dst) noexcept -> bool {
    static constexpr auto MAX = max_value<T>();

    const auto sign = this->extract_sign();
    if (sign == 0 || sign == '-') {
      return false;
    }

    const auto radix = this->extract_radix();
    if (radix == 0) {
      return false;
    }

    if (_ptr == _end) {
      return false;
    }
    const auto uval = this->extract_uint(radix, MAX);
    if (_ptr != _end) {
      return false;
    }

    dst = static_cast<T>(uval);
    return true;
  }

  template <trait::sint_ T>
  auto read(T& dst) noexcept -> bool {
    static constexpr auto MAX = static_cast<u64>(max_value<T>()) + 1;

    const auto sign = this->extract_sign();
    if (sign == 0) {
      return false;
    }
    const auto radix = this->extract_radix();
    if (radix == 0) {
      return false;
    }

    if (_ptr == _end) {
      return false;
    }
    const auto uval = this->extract_uint(radix, MAX);
    if (_ptr != _end) {
      return false;
    }

    if (uval == MAX && sign != '-') {
      return false;
    }

    dst = sign == '-' ? -static_cast<T>(uval) : static_cast<T>(uval);
    return true;
  }

 private:
  auto extract_sign() noexcept -> char {
    if (_ptr == _end) {
      return 0;
    }
    const auto c = _ptr[0];
    if (c == '+' || c == '-') {
      ++_ptr;
    }
    return c;
  }

  auto extract_radix() noexcept -> u32 {
    if (_ptr == _end) {
      return 0;
    }
    if (_ptr + 1 >= _end) {
      return 10;
    }

    if (_ptr[0] != '0') {
      return 10;
    }
    ++_ptr;

    switch (_ptr[0]) {
      case 'b':
      case 'B': ++_ptr; return 2;
      case 'x':
      case 'X': ++_ptr; return 16;
      default:  return 8;
    }
  }

  auto extract_uint(u32 radix, u64 max_val) -> u64 {
    const auto A = max_val / radix;
    const auto B = max_val % radix;

    auto val = u64{0};
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

auto to_str(slice::Slice<char> buf, trait::int_ auto val, char type) noexcept -> str::Str {
  auto imp = IntFmter{buf._ptr, buf._ptr + buf._len};
  return imp.write(val + 0, type);
}

auto from_str(str::Str buf, trait::int_ auto& val) noexcept -> bool {
  auto imp = IntParser{buf._ptr, buf._ptr + buf._len};
  return imp.read(val);
}

template auto to_str(slice::Slice<char>, signed char, char) noexcept -> str::Str;
template auto to_str(slice::Slice<char>, short, char) noexcept -> str::Str;
template auto to_str(slice::Slice<char>, int, char) noexcept -> str::Str;
template auto to_str(slice::Slice<char>, long, char) noexcept -> str::Str;
template auto to_str(slice::Slice<char>, long long, char) noexcept -> str::Str;

template auto to_str(slice::Slice<char>, unsigned char, char) noexcept -> str::Str;
template auto to_str(slice::Slice<char>, unsigned short, char) noexcept -> str::Str;
template auto to_str(slice::Slice<char>, unsigned int, char) noexcept -> str::Str;
template auto to_str(slice::Slice<char>, unsigned long, char) noexcept -> str::Str;
template auto to_str(slice::Slice<char>, unsigned long long, char) noexcept -> str::Str;

template auto from_str(str::Str, signed char&) noexcept -> bool;
template auto from_str(str::Str, short&) noexcept -> bool;
template auto from_str(str::Str, int&) noexcept -> bool;
template auto from_str(str::Str, long&) noexcept -> bool;
template auto from_str(str::Str, long long&) noexcept -> bool;

template auto from_str(str::Str, unsigned char&) noexcept -> bool;
template auto from_str(str::Str, unsigned short&) noexcept -> bool;
template auto from_str(str::Str, unsigned int&) noexcept -> bool;
template auto from_str(str::Str, unsigned long&) noexcept -> bool;
template auto from_str(str::Str, unsigned long long&) noexcept -> bool;

}  // namespace sfc::num
