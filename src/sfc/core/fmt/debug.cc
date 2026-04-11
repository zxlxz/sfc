

#include "sfc/core/fmt/debug.h"

namespace sfc::fmt {

struct RevBuff {
  char* _buf;
  usize _cap;
  char _type = 0;
  usize _pos = _cap;

 public:
  auto as_str() const -> Str {
    return Str{_buf + _pos, _cap - _pos};
  }

  void push(char c) {
    if (_pos == 0) return;
    _buf[--_pos] = c;
  }

  void write_dec(auto val) noexcept {
    static const char DIGITS[] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";

    // write digits
    auto uval = val > 0 ? val : 0 - val;
    for (; uval >= 100; uval /= 100) {
      const auto n = uval % 100 * 2;
      this->push(DIGITS[n + 1]);
      this->push(DIGITS[n + 0]);
    }
    if (uval >= 10) {
      const auto n = uval % 100 * 2;
      this->push(DIGITS[n + 1]);
      this->push(DIGITS[n + 0]);
    } else {
      this->push(static_cast<char>(uval + '0'));
    }

    // write sign
    if constexpr (num::sint_<decltype(val)>) {
      if (val < 0) this->push('-');
    }
  }

  template <u32 RADIX>
  void write_bin(auto val) noexcept {
    static constexpr auto UPCASE = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static constexpr auto LOWCASE = "0123456789abcdefghijklmnopqrstuvwxyz";
    static constexpr auto MASK = RADIX - 1;
    static_assert((RADIX & (RADIX - 1)) == 0, "radix must be power of 2");

    // write digits
    const auto DIGITS = _type < 'a' ? UPCASE : LOWCASE;
    auto uval = val > 0 ? val : 0 - val;
    if (uval == 0) {
      this->push('0');
    } else {
      for (; uval != 0; uval /= RADIX) {
        this->push(DIGITS[uval & MASK]);
      }
    }

    // write sign
    if constexpr (num::sint_<decltype(val)>) {
      if (val < 0) this->push('-');
    }
  }

  void write_ptr(usize uval) noexcept {
    static const auto MIN_LEN = 12L;

    this->write_bin<16>(uval);
    while (_cap - _pos < MIN_LEN) {
      this->push('0');
    }
    this->push(_type < 'a' ? 'X' : 'x');
    this->push('0');
  }

  void write_flt(u64 int_part, u64 flt_part) noexcept {
    if (flt_part >= 10) {
      this->write_dec(flt_part);
      _buf[_pos] = '.';
    }
    this->write_dec(int_part);
  }

  void write_exp(i32 exp_part) noexcept {
    const auto uexp = exp_part > 0 ? exp_part : -exp_part;
    this->write_dec(uexp);
    if (uexp < 10) {
      this->push('0');
    }
    this->push(exp_part < 0 ? '-' : '+');
    this->push(_type < 'a' ? 'E' : 'e');
  }
};

struct Float {
  u64 _int;
  u64 _flt;

 public:
  static auto pow10(u32 val) noexcept -> u64 {
    auto res = u64{1U};
    while (val > 0) {
      res *= 10U;
      --val;
    }
    return res;
  }

  static auto from(f64 uval, u32 precision) -> Float {
    // extract integer part
    const auto int_val = static_cast<u64>(uval);
    const auto flt_val = uval - static_cast<f64>(int_val);

    // extract fractional part
    const auto exp_val = pow10(precision);
    auto int_part = int_val;
    auto flt_part = static_cast<u64>(__builtin_round(flt_val * exp_val));
    if (flt_part >= exp_val) {
      flt_part -= exp_val;
      ++int_part;
    }
    flt_part += exp_val;
    return {int_part, flt_part};
  }
};

auto Debug::format_int(Slice<char> buf, auto val, char type) -> Str {
  auto rbuf = RevBuff{buf._ptr, buf._len, type ? type : 'd'};
  switch (type) {
    default:  rbuf.write_dec(val + 0); break;
    case 'B': rbuf.write_bin<2>(val + 0); break;
    case 'b': rbuf.write_bin<2>(val + 0); break;
    case 'O': rbuf.write_bin<8>(val + 0); break;
    case 'o': rbuf.write_bin<8>(val + 0); break;
    case 'X': rbuf.write_bin<16>(val + 0); break;
    case 'x': rbuf.write_bin<16>(val + 0); break;
  }
  return rbuf.as_str();
}

auto Debug::format_ptr(Slice<char> buf, auto ptr, char type) -> Str {
  const auto uval = reinterpret_cast<usize>(ptr);

  auto rbuf = RevBuff{buf._ptr, buf._len, type ? type : 'p'};
  switch (type) {
    default:  rbuf.write_ptr(uval); break;
    case 'X': rbuf.write_bin<16>(uval); break;
    case 'x': rbuf.write_bin<16>(uval); break;
    case 'p': rbuf.write_ptr(uval); break;
    case 'P': rbuf.write_ptr(uval); break;
  }
  return rbuf.as_str();
}

auto Debug::format_flt(Slice<char> buf, auto val, u32 precision, char type) -> Str {
  if (__builtin_isnan(val)) {
    return "nan";
  }
  if (__builtin_isinf(val)) {
    return val > 0 ? Str{"inf"} : Str{"-inf"};
  }
  auto uval = val >= 0 ? val : -val;

  auto rbuf = RevBuff{buf._ptr, buf._len, type ? type : 'f'};
  if (type == 'e' || type == 'E') {
    auto exp = 0;
    if (uval != 0) {
      while (uval >= 10.0) {
        uval /= 10.0;
        ++exp;
      }
      while (uval < 1.0) {
        uval *= 10.0;
        --exp;
      }
    }
    rbuf.write_exp(exp);
  }

  const auto u = Float::from(uval, precision);
  rbuf.write_flt(u._int, u._flt);
  if (val < 0) {
    rbuf.push('-');
  }
  return rbuf.as_str();
}

template auto Debug::format_int(Slice<char> buf, int val, char type) -> Str;
template auto Debug::format_int(Slice<char> buf, long val, char type) -> Str;
template auto Debug::format_int(Slice<char> buf, long long val, char type) -> Str;

template auto Debug::format_int(Slice<char> buf, unsigned val, char type) -> Str;
template auto Debug::format_int(Slice<char> buf, unsigned long val, char type) -> Str;
template auto Debug::format_int(Slice<char> buf, unsigned long long val, char type) -> Str;

template auto Debug::format_ptr(Slice<char> buf, const void* val, char type) -> Str;

template auto Debug::format_flt(Slice<char> buf, float val, u32 precision, char type) -> Str;
template auto Debug::format_flt(Slice<char> buf, double val, u32 precision, char type) -> Str;

}  // namespace sfc::fmt
