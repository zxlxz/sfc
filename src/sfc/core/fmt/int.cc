#include "sfc/core/fmt.h"

namespace sfc::fmt {

struct IntBuf {
  char* _start;
  char* _end;
  char* _ptr = _end;

 public:
  auto as_str() const -> Str {
    return {_ptr, static_cast<usize>(_end - _ptr)};
  }

  void fill(auto val, char type) {
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
  }

 private:
  void push(char c) {
    if (_ptr == _start) {
      return;
    }
    *--_ptr = c;
  }

  void write_dec(auto val) {
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
  void write_bin(auto val) {
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
      const auto kCurLen = static_cast<usize>(_end - _ptr);
      for (auto i = kCurLen; i < kMinLen; ++i) {
        this->push('0');
      }
    }
  }
};

auto Debug::to_str(Slice<char> buf, trait::int_ auto val, Style style) noexcept -> Str {
  auto ss = IntBuf{buf._ptr, buf._ptr + buf._len};
  ss.fill(val, style.type());
  return ss.as_str();
}

auto Debug::to_str(Slice<char> buf, const void* ptr, Style style) noexcept -> Str {
  const auto val = __builtin_bit_cast(usize, ptr);
  auto ss = IntBuf{buf._ptr, buf._ptr + buf._len};
  ss.fill(val, style.type() ? style.type() : 'p');
  return ss.as_str();
}

template auto Debug::to_str(Slice<char>, signed char, Style) noexcept -> Str;
template auto Debug::to_str(Slice<char>, short, Style) noexcept -> Str;
template auto Debug::to_str(Slice<char>, int, Style) noexcept -> Str;
template auto Debug::to_str(Slice<char>, long, Style) noexcept -> Str;
template auto Debug::to_str(Slice<char>, long long, Style) noexcept -> Str;

template auto Debug::to_str(Slice<char>, unsigned char, Style) noexcept -> Str;
template auto Debug::to_str(Slice<char>, unsigned short, Style) noexcept -> Str;
template auto Debug::to_str(Slice<char>, unsigned int, Style) noexcept -> Str;
template auto Debug::to_str(Slice<char>, unsigned long, Style) noexcept -> Str;
template auto Debug::to_str(Slice<char>, unsigned long long, Style) noexcept -> Str;

}  // namespace sfc::fmt
