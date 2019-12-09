#include "rc.inl"

#include "rc/core/fmt/num.h"
#include "rc/core/macros.h"

#ifndef __clang__
#define __builtin_snprintf ::snprintf
#endif

namespace rc::fmt {

static u32 _ZERO_OR_POWERS_OF_10_U32[] = {
    0, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000};

static u64 _ZERO_OR_POWERS_OF_10_U64[] = {0,
                                          10,
                                          100,
                                          1000,
                                          10000,
                                          100000,
                                          1000000,
                                          10000000,
                                          100000000,
                                          1000000000,
                                          10000000000,
                                          100000000000,
                                          1000000000000,
                                          10000000000000,
                                          100000000000000,
                                          1000000000000000,
                                          10000000000000000,
                                          100000000000000000,
                                          1000000000000000000};

template <usize BITS, class UInt>
auto count_digits_by_bits(UInt val) -> u32 {
  const auto nbits = 32u - intrin::ctlz(val);
  return (nbits + BITS - 1) / BITS;
}

template<class UInt>
auto count_digits_radix_10(UInt val) -> u32 {
  if constexpr (sizeof(UInt) <= sizeof(u32)) {
    const auto t = ((32u - intrin::ctlz(val | 1)) * 1233u) >> 12u;
    const auto u = u32(val) < _ZERO_OR_POWERS_OF_10_U32[t] ? 0u : 1u;
    return t + u;
  }
  if constexpr (sizeof(UInt) == sizeof(u64)) {
    const auto t = ((64u - intrin::ctlz(val | 1)) * 1233u) >> 12u;
    const auto u = u64(val) < _ZERO_OR_POWERS_OF_10_U64[t] ? 0u : 1u;
    return t + u;
  }
}

template <usize RADIX, class UInt>
auto count_digits_by_radix(UInt val) -> u32 {
  if constexpr (RADIX == 2) {
    return fmt::count_digits_by_bits<1>(val);
  }
  if constexpr (RADIX == 8) {
    return fmt::count_digits_by_bits<3>(val);
  }
  if constexpr (RADIX == 16) {
    return fmt::count_digits_by_bits<4>(val);
  }
  if constexpr (RADIX == 10) {
    return fmt::count_digits_radix_10(val);
  }
}

template <usize RADIX, u8 TYPE, class UInt>
auto uint_fmt_with_radix(UInt val, Slice<u8> buf) -> Str {
  const auto cnt = fmt::count_digits_by_radix<RADIX>(val);
  if (cnt > buf._len) return u8"";

  auto end = buf._ptr + cnt;
  if constexpr (RADIX == 2) {
    do {
      --*end = u8('0' + (val & 0x1));
    } while ((val >>= 1) != 0);
  }
  if constexpr (RADIX == 8) {
    do {
      --*end = u8('0' + (val & 0x7));
    } while ((val >>= 3) != 0);
  }
  if constexpr (RADIX == 16) {
    const auto digits =
        (TYPE == u8'X') ? u8"0123456789ABCDEF" : u8"0123456789abcdef";

    do {
      --*end = digits[val & 0xF];
    } while ((val >>= 4) != 0);
  }
  if constexpr (RADIX == 10) {
    const auto digits =
        u8"0001020304050607080910111213141516171819"
        u8"2021222324252627282930313233343536373839"
        u8"4041424344454647484950515253545556575859"
        u8"6061626364656667686970717273747576777879"
        u8"8081828384858687888990919293949596979899";

    for (; val >= 100; val /= 100) {
      auto idx = (val % 100) * 2;
      *--end = digits[idx + 1];
      *--end = digits[idx];
    }
    if (val < 10) {
      *--end = UInt(u8'0' + u8(val));
    } else {
      *--end = digits[val * 2 + 1];
      *--end = digits[val * 2 + 0];
    }
  }
  return Str{buf._ptr, cnt};
}

template<class UInt>
pub auto int_to_str(UInt val, u8 type, Slice<u8> buf) -> Str {
  switch(type) {
    case u8'b':
      return fmt::uint_fmt_with_radix<2, u8'b'>(val, buf);
    case u8'B':
      return fmt::uint_fmt_with_radix<2, u8'B'>(val, buf);
    case u8'o':
      return fmt::uint_fmt_with_radix<8, u8'o'>(val, buf);
    case u8'O':
      return fmt::uint_fmt_with_radix<8, u8'O'>(val, buf);
    case u8'x':
      return fmt::uint_fmt_with_radix<16, u8'x'>(val, buf);
    case u8'X':
      return fmt::uint_fmt_with_radix<16, u8'X'>(val, buf);
    default:
      return fmt::uint_fmt_with_radix<10, u8'\0'>(val, buf);
  }
}

template pub auto int_to_str(u8 val, u8 type, Slice<u8> buf) -> Str;
template pub auto int_to_str(u16 val, u8 type, Slice<u8> buf) -> Str;
template pub auto int_to_str(u32 val, u8 type, Slice<u8> buf) -> Str;
template pub auto int_to_str(u64 val, u8 type, Slice<u8> buf) -> Str;

template pub auto int_to_str(i8 val, u8 type, Slice<u8> buf) -> Str;
template pub auto int_to_str(i16 val, u8 type, Slice<u8> buf) -> Str;
template pub auto int_to_str(i32 val, u8 type, Slice<u8> buf) -> Str;
template pub auto int_to_str(i64 val, u8 type, Slice<u8> buf) -> Str;

pub auto flt_to_str(f64 val, u8 type, u32 prec, Slice<u8> buf) -> Str {
  const auto p = ptr::cast<char>(buf._ptr);
  const auto n = buf.len();

  const auto cnt = [=] {
    switch (type) {
      default:
        return __builtin_snprintf(p, n, "%.*f", prec, val);
      case 'e':
        return __builtin_snprintf(p, n, "%.*e", prec, val);
      case 'g':
        return __builtin_snprintf(p, n, "%.*g", prec, val);
    }
  }();

  if (cnt <= 0) return u8"";

  return Str{buf._ptr, usize(cnt)};
}


}  // namespace rc::fmt
