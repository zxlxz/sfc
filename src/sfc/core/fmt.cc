#include "sfc/core/fmt.h"
#include "sfc/core/num.h"

namespace sfc::fmt {

static auto exp10(u32 n) -> f64 {
  static constexpr f64 kTBL[] = {
      1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
      1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29, 1e30, 1e31,
  };

  if (n == 0) {
    return 1.0;
  }

  auto x = 1.0;
  for (; n > 32; n -= 32) {
    x *= 1e32;
  }
  x *= kTBL[n];
  return x;
}

struct FltPoint {
  u64 _int;  // int part
  u64 _flt;  // flt part

 public:
  static auto build(f64 t, u32 precision) -> FltPoint {
    static constexpr auto kMaxPrecision = 20U;
    if (precision > kMaxPrecision) {
      precision = kMaxPrecision;
    }

    const auto exp_val = fmt::exp10(precision);

    // extract integer part
    auto int_val = __builtin_floor(t);
    auto flt_val = __builtin_round((t - int_val) * exp_val);
    if (flt_val >= exp_val) {
      flt_val -= exp_val;
      int_val += 1;
    }

    // add 1000... to flt_val
    // to ensure it has leading zeros when converted to string
    flt_val += exp_val;

    const auto int_part = num::trunc_to_int(int_val);
    const auto flt_part = num::trunc_to_int(flt_val);

    return FltPoint{
        num::cast_unsigned(int_part),
        num::cast_unsigned(flt_part),
    };
  }
};

struct FixPoint {
  u64 _fix;  // fixed point value
  i32 _exp;  // exponent, the actual value is _fix * 10^(-_exp)

 public:
  static auto unpack_exp(f64 t) -> i32 {
    auto n = 0;
    if (t > 10.0) {
      for (; t >= 1e10; n += 10) {
        t *= 1e-10;
      }
      for (; t >= 10.0; n += 1) {
        t *= 1e-1;
      }
      return n;
    }

    if (t < 1.00) {
      if (t < 1e-200) {  // too small, treat as zero
        return 0;
      }
      for (; t <= 1e-10; n -= 10) {
        t *= 1e10;
      }
      for (; t < 1.00; n -= 1) {
        t *= 1e1;
      }
      return n;
    }
    return 0;
  }

  static auto build(f64 t, u32 precision) -> FixPoint {
    static constexpr auto kMaxPrecision = 20U;
    if (precision > kMaxPrecision) {
      precision = kMaxPrecision;
    }

    const auto exp_cnt = FixPoint::unpack_exp(t);
    const auto exp_factor = fmt::exp10(num::unsigned_abs(exp_cnt));
    const auto uval = exp_cnt >= 0 ? t / exp_factor : t * exp_factor;

    const auto prec_ratio = fmt::exp10(precision);
    const auto fix_sint = num::trunc_to_int(uval * prec_ratio);
    const auto fix_uint = num::cast_unsigned(fix_sint);
    if (f64(fix_sint) > prec_ratio * 10 && exp_cnt > 0) {
      return FixPoint{fix_uint / 10, exp_cnt - 1};
    }

    return FixPoint{fix_uint, exp_cnt};
  }
};

struct RevBuf {
  char* _buf;
  usize _cap;
  usize _pos = _cap;

 public:
  RevBuf(Slice<char> buf) noexcept : _buf{buf._ptr}, _cap{buf._len} {}

  auto as_str() const -> Str {
    return Str{_buf + _pos, _cap - _pos};
  }

  auto write_u64(u64 uval, char type = 0) noexcept -> Str {
    switch (type) {
      default:  this->write_dec(uval); break;
      case 'B': this->write_bin<2>(uval); break;
      case 'b': this->write_bin<2>(uval); break;
      case 'O': this->write_bin<8>(uval); break;
      case 'o': this->write_bin<8>(uval); break;
      case 'X': this->write_bin<16>(uval, true); break;
      case 'x': this->write_bin<16>(uval, false); break;
    }
    return this->as_str();
  }

  auto write_ptr(usize uval, char p = 'x') noexcept -> Str {
    static const auto MIN_LEN = 12L;

    this->write_bin<16>(uval);
    while (_cap - _pos < MIN_LEN) {
      this->push('0');
    }
    this->push(p <= 'a' ? 'x' : 'X');
    this->push('0');
    return this->as_str();
  }

  auto write_flt(f64 val, u32 precision, char type) -> Str {
    static constexpr auto kMaxFlt = 1e30;
    const auto use_exp = type == 'e' || type == 'E' || val > kMaxFlt;

    if (__builtin_isnan(val)) {
      return "nan";
    }
    if (__builtin_isinf(val)) {
      return val > 0 ? Str{"inf"} : Str{"-inf"};
    }

    const auto uval = __builtin_fabs(val);
    if (use_exp) {
      const auto imp = FixPoint::build(uval, precision);
      this->write_fix(imp, type);
    } else {
      const auto imp = FltPoint::build(uval, precision);
      this->write_flt(imp);
    }
    if (val < 0) {
      this->push('-');
    }
    return this->as_str();
  }

 private:
  void push(char c) {
    if (_pos == 0) return;
    _buf[--_pos] = c;
  }

  void write_dec(u64 val) noexcept {
    static const char DIGITS_1[] = "0123456789";
    static const char DIGITS_2[] =
        "0001020304050607080910111213141516171819"
        "2021222324252627282930313233343536373839"
        "4041424344454647484950515253545556575859"
        "6061626364656667686970717273747576777879"
        "8081828384858687888990919293949596979899";

    // write digits
    auto uval = val > 0 ? val : 0 - val;
    for (; uval >= 100; uval /= 100) {
      const auto n = uval % 100 * 2;
      this->push(DIGITS_2[n + 1]);
      this->push(DIGITS_2[n + 0]);
    }
    if (uval >= 10) {
      const auto n = uval % 100 * 2;
      this->push(DIGITS_2[n + 1]);
      this->push(DIGITS_2[n + 0]);
    } else {
      this->push(DIGITS_1[uval]);
    }
  }

  template <u32 RADIX>
  void write_bin(u64 val, bool upcase = false) noexcept {
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
    if constexpr (trait::sint_<decltype(val)>) {
      if (val < 0) this->push('-');
    }
  }

  void write_flt(FltPoint flt) noexcept {
    if (flt._flt >= 10) {
      this->write_dec(flt._flt);
      _buf[_pos] = '.';
    }
    this->write_dec(flt._int);
  }

  void write_fix(FixPoint fix, char type = 'e') noexcept {
    const auto exp_sign = fix._exp < 0 ? '-' : '+';
    const auto exp_char = type < 'a' ? 'E' : 'e';
    const auto uexp = num::unsigned_abs(fix._exp);
    this->write_dec(uexp);
    if (uexp < 10) {
      this->push('0');
    }
    this->push(exp_sign);
    this->push(exp_char);
    this->write_dec(fix._fix);
    if (fix._fix >= 10) {
      const auto c = _buf[_pos];
      _buf[_pos] = '.';
      this->push(c);
    }
  }
};

auto Spec::sign(bool is_neg) const -> str::Str {
  if (is_neg) {
    return "-";
  }

  switch (_sign) {
    case '+': return "+";
    case '-': return " ";
    default:  return "";
  }
}

auto Spec::prefix() const -> str::Str {
  if (_prefix != '#') {
    return "";
  }

  switch (_type) {
    case 'O':
    case 'o': return "0";
    case 'B': return "0B";
    case 'b': return "0b";
    case 'X': return "0X";
    case 'x': return "0x";
    case 'P': return "0X";
    case 'p': return "0x";
    default:  return "";
  }
}

void Debug::fmt(bool val, Formatter& f) {
  f.write_str(val ? Str{"true"} : Str{"false"});
}

void Debug::fmt(char val, Formatter& f) {
  f.write_char(val);
}

void Debug::fmt(char16_t val, Formatter& f) {
  u8 u8_buf[4] = {};
  const auto u8_len = chr::utf8_encode(u8_buf, val);
  f.write_str(Str::from_utf8({u8_buf, u8_len}));
}

void Debug::fmt(char32_t val, Formatter& f) {
  u8 u8_buf[4] = {};
  const auto u8_len = chr::utf8_encode(u8_buf, val);
  f.write_str(Str::from_utf8({u8_buf, u8_len}));
}

void Debug::fmt(unsigned int val, Formatter& f) {
  char buf[8 * sizeof(val) + 8];
  const auto s = RevBuf{buf}.write_u64(val, f.type());
  f.pad_num(false, s);
}

void Debug::fmt(unsigned long val, Formatter& f) {
  char buf[8 * sizeof(val) + 8];
  const auto s = RevBuf{buf}.write_u64(val, f.type());
  f.pad_num(false, s);
}

void Debug::fmt(unsigned long long val, Formatter& f) {
  char buf[8 * sizeof(val) + 8];
  const auto s = RevBuf{buf}.write_u64(val, f.type());
  f.pad_num(false, s);
}

void Debug::fmt(int val, Formatter& f) {
  const auto uval = num::unsigned_abs(val);

  char buf[8 * sizeof(val) + 8];
  const auto s = RevBuf{buf}.write_u64(uval, f.type());
  f.pad_num(val < 0, s);
}

void Debug::fmt(long val, Formatter& f) {
  const auto uval = num::unsigned_abs(val);

  char buf[8 * sizeof(val) + 8];
  const auto s = RevBuf{buf}.write_u64(uval, f.type());
  f.pad_num(val < 0, s);
}

void Debug::fmt(long long val, Formatter& f) {
  const auto uval = num::unsigned_abs(val);

  char buf[8 * sizeof(val) + 8];
  const auto s = RevBuf{buf}.write_u64(uval, f.type());
  f.pad_num(val < 0, s);
}

void Debug::fmt(float val, Formatter& f) {
  static constexpr auto kDefautlPrecision = 4U;
  const auto uval = val < 0 ? -val : val;
  const auto prec = f.precision().unwrap_or(kDefautlPrecision);

  char buf[8 * sizeof(val) + 16];
  const auto s = RevBuf{buf}.write_flt(uval, prec, f.type());
  f.pad_num(val < 0, s);
}

void Debug::fmt(double val, Formatter& f) {
  static constexpr auto kDefautlPrecision = 6U;
  const auto uval = val < 0 ? -val : val;
  const auto prec = f.precision().unwrap_or(kDefautlPrecision);

  char buf[8 * sizeof(val) + 16];
  const auto s = RevBuf{buf}.write_flt(uval, prec, f.type());
  f.pad_num(val < 0, s);
}

void Debug::fmt(const void* val, Formatter& f) {
  const auto uval = __builtin_bit_cast(usize, val);

  char buf[16];
  const auto s = RevBuf{buf}.write_ptr(uval, f.type());
  f.write_str(s);
}

void Formatter::write_chars(char c, usize n) {
  static constexpr auto BUF_LEN = 8U;
  const char buf[BUF_LEN] = {c, c, c, c, c, c, c, c};
  for (auto i = 0U; i < n; i += BUF_LEN) {
    const auto s = Str{buf, i + BUF_LEN < n ? BUF_LEN : n - i};
    this->write_str(s);
  }
}

void Formatter::pad(Str s) {
  if (_spec._width <= s._len) {
    this->write_str(s);
    return;
  }

  const auto fill = _spec.fill();
  const auto npad = _spec._width - s._len;

  switch (_spec._align) {
    default:
    case '<':
      this->write_str(s);
      this->write_chars(fill, npad);
      break;
    case '>':
      this->write_chars(fill, npad);
      this->write_str(s);
      break;
    case '=':
    case '^':
      this->write_chars(fill, (npad + 0) / 2);
      this->write_str(s);
      this->write_chars(fill, (npad + 1) / 2);
      break;
  }
}

void Formatter::pad_num(bool is_neg, Str num_str) {
  const auto sign = _spec.sign(is_neg);
  const auto prefix = _spec.prefix();
  if (num_str._len >= _spec._width) {
    this->write_str(sign);
    this->write_str(prefix);
    this->write_str(num_str);
    return;
  }

  const auto npad = _spec._width - num_str._len;
  const auto fill = _spec.fill(_spec._prefix ? '0' : ' ');
  const auto align = _spec.align(fill == '0' ? '=' : '>');
  switch (align) {
    default:
    case '>':
      this->write_chars(fill, npad);
      this->write_str(sign);
      this->write_str(prefix);
      this->write_str(num_str);
      break;
    case '<':
      this->write_str(sign);
      this->write_str(prefix);
      this->write_str(num_str);
      this->write_chars(fill, npad);
      break;
    case '=':
      this->write_str(sign);
      this->write_str(prefix);
      this->write_chars(fill, npad);
      this->write_str(num_str);
      break;
    case '^':
      this->write_chars(fill, (npad + 0) / 2);
      this->write_str(sign);
      this->write_str(prefix);
      this->write_str(num_str);
      this->write_chars(fill, (npad + 1) / 2);
  }
}

auto Formatter::debug_tuple() -> DebugTuple {
  return DebugTuple{*this};
}

auto Formatter::debug_list() -> DebugList {
  return DebugList{*this};
}

auto Formatter::debug_set() -> DebugSet {
  return DebugSet{*this};
}

auto Formatter::debug_map() -> DebugMap {
  return DebugMap{*this};
}

auto Formatter::debug_struct() -> DebugStruct {
  return DebugStruct{*this};
}

DebugList::DebugList(Formatter& fmt) : _fmt{fmt} {
  _fmt.write_str("[");
}

DebugList::~DebugList() {
  _fmt.write_str("]");
}

void DebugList::push(Str s) {
  if (_cnt++ != 0) _fmt.write_str(", ");
  _fmt.write_str(s);
}

DebugSet::DebugSet(Formatter& fmt) : _fmt{fmt} {
  _fmt.write_str("{");
}

DebugSet::~DebugSet() {
  _fmt.write_str("}");
}

void DebugSet::push(Str s) {
  if (_cnt++ != 0) _fmt.write_str(", ");
  _fmt.write_str(s);
}

DebugMap::DebugMap(Formatter& fmt) : _fmt{fmt} {
  _fmt.write_str("{");
}

DebugMap::~DebugMap() {
  _fmt.write_str("}");
}

void DebugMap::push(Str key, Str value) {
  if (_cnt++ != 0) _fmt.write_str(", ");
  _fmt.write_str("\"");
  _fmt.write_str(key);
  _fmt.write_str("\": ");
  _fmt.write_str(value);
}

DebugTuple::DebugTuple(Formatter& fmt) : _fmt{fmt} {
  _fmt.write_str("(");
}

DebugTuple::~DebugTuple() {
  _fmt.write_str(")");
}

void DebugTuple::push(Str s) {
  if (_cnt++ != 0) _fmt.write_str(", ");
  _fmt.write_str(s);
}

DebugStruct::DebugStruct(Formatter& fmt) : _fmt{fmt} {
  _fmt.write_str("{");
}

DebugStruct::~DebugStruct() {
  _fmt.write_str("}");
}

void DebugStruct::push(Str key, Str value) {
  if (_cnt++ != 0) _fmt.write_str(", ");
  _fmt.write_str(key);
  _fmt.write_str(": ");
  _fmt.write_str(value);
}

}  // namespace sfc::fmt
