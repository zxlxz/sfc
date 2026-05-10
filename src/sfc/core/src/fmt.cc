#include "sfc/core/fmt.h"
#include "sfc/core/num.h"

namespace sfc::fmt {

struct RevBuf {
  char* _buf;
  usize _cap;
  usize _pos = _cap;

 public:
  RevBuf(Slice<char> buf) noexcept : _buf{buf._ptr}, _cap{buf._len} {}

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
  void write_bin(auto val, bool upcase = false) noexcept {
    static constexpr auto UPCASE = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    static constexpr auto LOWCASE = "0123456789abcdefghijklmnopqrstuvwxyz";
    static constexpr auto MASK = RADIX - 1;
    static_assert((RADIX & (RADIX - 1)) == 0, "radix must be power of 2");

    // write digits
    const auto DIGITS = upcase ? UPCASE : LOWCASE;
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

  void write_ptr(usize uval, bool upcase = false) noexcept {
    static const auto MIN_LEN = 12L;

    this->write_bin<16>(uval);
    while (_cap - _pos < MIN_LEN) {
      this->push('0');
    }
    this->push(upcase ? 'X' : 'x');
    this->push('0');
  }

  void write_flt(u64 int_part, u64 flt_part) noexcept {
    if (flt_part >= 10) {
      this->write_dec(flt_part);
      _buf[_pos] = '.';
    }
    this->write_dec(int_part);
  }

  void write_exp(i32 exp_part, char type = 'e') noexcept {
    const auto uexp = exp_part > 0 ? exp_part : -exp_part;
    this->write_dec(uexp);
    if (uexp < 10) {
      this->push('0');
    }
    this->push(exp_part < 0 ? '-' : '+');
    this->push(type);
  }
};

struct Decimal {
  u64 _int;
  u64 _flt;
  i32 _exp = 0;

 public:
  static auto unpack_exp(f64* pu) -> i32 {
    if (pu == nullptr) {
      return 0;
    }

    auto& u = *pu;
    if (u == 0.0) {
      return 0;
    }

    auto n = 0;
    if (u >= 1e1) {
      for (; u >= 1e4; n += 4) {
        u /= 1e4;
      }
      for (; u >= 1e1; n += 1) {
        u /= 1e1;
      }
      return n;
    }

    if (u < 1e0) {
      for (; u < 1e-4; n -= 4) {
        u *= 1e4;
      }
      for (; u < 1e0; n -= 1) {
        u *= 1e1;
      }
      return n;
    }

    return 0;
  }

  static constexpr u32 kPow10Len = 20U;
  static constexpr f64 kPow10Tbl[] = {
      1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,
      1e10, 1e11, 1e12, 1e13, 1e14, 1e15, 1e16, 1e17, 1e18, 1e19,
  };

  static auto unpack(f64 uval, u32 precision, bool use_exp = false) -> Decimal {
    // limit precision to 20, otherwise may overflow u64
    if (precision >= kPow10Len) {
      precision = kPow10Len - 1;
    }

    // extract integer part
    auto exp_part = use_exp ? unpack_exp(&uval) : 0;
    auto int_part = static_cast<u64>(uval);

    const auto flt_val = uval - static_cast<f64>(int_part);
    const auto exp_val = kPow10Tbl[precision];
    auto flt_part = static_cast<u64>(flt_val * exp_val + 0.5);

    if (flt_part >= exp_val) {
      flt_part -= exp_val;
      int_part += 1;

      // in exp format, if int_part becomes 10
      // it should be converted to 1 and exp_part should be increased by 1
      if (use_exp && int_part == 10) {
        int_part = 1;
        exp_part += 1;
      }
    }

    // add 100..00 to flt_part,
    // to ensure it has leading zeros when converted to string
    flt_part += exp_val;
    return {int_part, flt_part, exp_part};
  }
};

auto Debug::format_int(Slice<char> buf, auto val, char type) -> Str {
  const auto uval = num::uabs(val) + 0U;

  auto rbuf = RevBuf{buf};
  switch (type) {
    default:  rbuf.write_dec(uval); break;
    case 'B': rbuf.write_bin<2>(uval); break;
    case 'b': rbuf.write_bin<2>(uval); break;
    case 'O': rbuf.write_bin<8>(uval); break;
    case 'o': rbuf.write_bin<8>(uval); break;
    case 'X': rbuf.write_bin<16>(uval, true); break;
    case 'x': rbuf.write_bin<16>(uval, false); break;
  }

  if constexpr (num::sint_<decltype(val)>) {
    if (val < 0) {
      rbuf.push('-');
    }
  }

  return rbuf.as_str();
}

auto Debug::format_ptr(Slice<char> buf, auto ptr, char type) -> Str {
  const auto uval = reinterpret_cast<usize>(ptr);

  auto rbuf = RevBuf{buf};
  switch (type) {
    default:  rbuf.write_ptr(uval); break;
    case 'X': rbuf.write_bin<16>(uval, true); break;
    case 'x': rbuf.write_bin<16>(uval, false); break;
    case 'P': rbuf.write_ptr(uval, true); break;
    case 'p': rbuf.write_ptr(uval, false); break;
  }
  return rbuf.as_str();
}

auto Debug::format_flt(Slice<char> buf, auto val, u32 precision, char type) -> Str {
  static constexpr auto kMaxFlt = 1e30;

  if (__builtin_isnan(val)) {
    return "nan";
  }
  if (__builtin_isinf(val)) {
    return val > 0 ? Str{"inf"} : Str{"-inf"};
  }

  auto uval = val >= 0 ? val : -val;
  auto enable_exp = type == 'E' || type == 'e' || val > kMaxFlt;
  const auto dcm = Decimal::unpack(uval, precision, enable_exp);

  auto rbuf = RevBuf{buf};
  if (enable_exp) {
    rbuf.write_exp(dcm._exp, type == 'E' ? 'E' : 'e');
  }
  rbuf.write_flt(dcm._int, dcm._flt);
  if (val < 0) {
    rbuf.push('-');
  }

  return rbuf.as_str();
}

template auto Debug::format_int(Slice<char> buf, signed char val, char type) -> Str;
template auto Debug::format_int(Slice<char> buf, short val, char type) -> Str;
template auto Debug::format_int(Slice<char> buf, int val, char type) -> Str;
template auto Debug::format_int(Slice<char> buf, long val, char type) -> Str;
template auto Debug::format_int(Slice<char> buf, long long val, char type) -> Str;

template auto Debug::format_int(Slice<char> buf, unsigned char val, char type) -> Str;
template auto Debug::format_int(Slice<char> buf, unsigned short val, char type) -> Str;
template auto Debug::format_int(Slice<char> buf, unsigned int val, char type) -> Str;
template auto Debug::format_int(Slice<char> buf, unsigned long val, char type) -> Str;
template auto Debug::format_int(Slice<char> buf, unsigned long long val, char type) -> Str;

template auto Debug::format_ptr(Slice<char> buf, const void* val, char type) -> Str;

template auto Debug::format_flt(Slice<char> buf, f32 val, u32 precision, char type) -> Str;
template auto Debug::format_flt(Slice<char> buf, f64 val, u32 precision, char type) -> Str;

template struct Formatter<FixedBuf<64>>;
template struct Formatter<FixedBuf<128>>;
template struct Formatter<FixedBuf<256>>;
template struct Formatter<FixedBuf<1024>>;
template struct Formatter<FixedBuf<2048>>;
template struct Formatter<FixedBuf<4096>>;

}  // namespace sfc::fmt
