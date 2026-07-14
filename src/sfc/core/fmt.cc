#include "sfc/core/fmt.h"
#include "sfc/core/num.h"

namespace sfc::fmt {

struct FixFloat {
  u64 _int;  // int part
  u64 _flt;  // flt part
};

struct ExpFloat {
  u64 _base;  // fixed point value
  i32 _iexp;  // exponent, the actual value is _val * 10^(-_exp)
};

static auto exp10(int n) -> f64 {
  constexpr u32 kCNT = 32U;
  constexpr f64 kTBL[] = {
      1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
      1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29, 1e30, 1e31,
  };

  auto u = n < 0 ? u32(-n) : u32(n);
  if (u == 0) {
    return 1.0;
  }

  auto x = 1.0;
  for (; u >= kCNT; u -= kCNT) {
    x *= 1e32;
  }
  x *= kTBL[u];

  if (n < 0) {
    x = 1.0 / x;
  }
  return x;
}

static auto frexp10(f64 uflt, int* exp) {
  auto e2 = 0;
  __builtin_frexp(uflt, &e2);

  auto e10 = int(e2 * 0.301029995663981195);
  auto m10 = uflt / exp10(e10);

  if (m10 >= 10.0) {
    m10 /= 10.0;
    e10 += 1;
  } else if (m10 < 1.0 && m10 != 0.0) {
    m10 *= 10.0;
    e10 -= 1;
  }

  *exp = e10;
  return m10;
}

static auto prec_ratio(u32 precision) -> u64 {
  const u64 k10 = u64(1e10);  // 1e10
  const u64 k19 = u64(1e19);  // 1e19
  const u64 kTBL[10] = {1U, 10U, 100U, 1000U, 10000U, 100000U, 1000000U, 10000000U, 100000000U, 1000000000U};
  if (precision < 10) return kTBL[precision];
  if (precision < 20) return k10 * kTBL[precision - 10];
  return k19;
}

static auto build_fix(f64 uflt, u32 precision) -> FixFloat {
  const auto kMaxPrecision = 20U;
  if (precision >= kMaxPrecision) {
    precision = kMaxPrecision - 1;
  }

  // 1000...
  const auto prec_ratio = fmt::prec_ratio(precision);

  // extract integer part
  auto int_part = u64(uflt);
  auto flt_part = u64(__builtin_round((uflt - int_part) * f64(prec_ratio)));
  if (flt_part >= prec_ratio) {
    flt_part -= prec_ratio;
    int_part += 1;
  }

  // add 1000... to flt_part
  // to ensure it has leading zeros when converted to string
  flt_part += prec_ratio;

  return FixFloat{int_part, flt_part};
}

static auto build_exp(f64 uflt, u32 precision) -> ExpFloat {
  const auto prec_ratio = fmt::prec_ratio(precision);

  auto exp_cnt = 0;
  auto flt_val = frexp10(uflt, &exp_cnt);

  auto int_part = u64(__builtin_round(flt_val * prec_ratio));
  if (int_part / 10 >= prec_ratio) {
    int_part /= 10;
    exp_cnt += 1;
  }

  return ExpFloat{u64(int_part), exp_cnt};
}

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
      default:  this->push_dec(uval); break;
      case 'B': this->push_bin<2>(uval); break;
      case 'b': this->push_bin<2>(uval); break;
      case 'O': this->push_bin<8>(uval); break;
      case 'o': this->push_bin<8>(uval); break;
      case 'X': this->push_bin<16>(uval, true); break;
      case 'x': this->push_bin<16>(uval, false); break;
    }
    return this->as_str();
  }

  auto write_ptr(usize uval, bool upcase) noexcept -> Str {
    static const auto MIN_LEN = 12L;

    this->push_bin<16>(uval, upcase);
    while (_cap - _pos < MIN_LEN) {
      this->push('0');
    }
    this->push(upcase ? 'X' : 'x');
    this->push('0');
    return this->as_str();
  }

  auto write_flt(f64 val, u32 precision, char type) -> Str {
    constexpr auto kMaxFix = f64(num::Int<u64>::MAX);

    if (__builtin_isnan(val)) {
      return "nan";
    }
    if (__builtin_isinf(val)) {
      return val > 0 ? Str{"inf"} : Str{"-inf"};
    }

    const auto uval = __builtin_fabs(val);
    const auto upcase = chr::is_upcase(type);
    const auto use_exp = type == 'e' || type == 'E' || uval >= kMaxFix;
    if (use_exp) {
      const auto [base, iexp] = build_exp(uval, precision);
      this->write_exp(base, iexp, upcase);
    } else {
      const auto [int_part, flt_part] = build_fix(uval, precision);
      this->write_fix(int_part, flt_part);
    }
    if (val < 0) {
      this->push('-');
    }
    return this->as_str();
  }

  void write_fix(u64 int_part, u64 flt_part) noexcept {
    if (flt_part >= 10) {
      this->push_dec(flt_part);
      _buf[_pos] = '.';
    }
    this->push_dec(int_part);
  }

  void write_exp(u64 base, i32 exp, bool upcase) noexcept {
    const auto uexp = exp < 0 ? u32(-exp) : u32(exp);
    this->push_dec(uexp);
    if (uexp < 10) this->push('0');
    this->push(exp < 0 ? '-' : '+');

    this->push(upcase ? 'E' : 'e');
    this->push_dec(base);

    if (base >= 10) {  // 12e3 -> 1.2e3
      const auto c = _buf[_pos];
      _buf[_pos] = '.';
      this->push(c);
    }
  }

 private:
  void push(char c) {
    if (_pos == 0) return;
    _buf[--_pos] = c;
  }

  void push_dec(u64 val) noexcept {
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
  void push_bin(u64 val, bool upcase = false) noexcept {
    static_assert((RADIX & (RADIX - 1)) == 0, "radix must be power of 2");
    constexpr auto MASK = RADIX - 1;

    const auto digits_upper = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const auto digits_lower = "0123456789abcdefghijklmnopqrstuvwxyz";

    // write digits
    const auto digits = upcase ? digits_upper : digits_lower;
    auto uval = val > 0 ? val : 0 - val;
    if (uval == 0) {
      this->push('0');
    } else {
      for (; uval != 0; uval /= RADIX) {
        this->push(digits[uval & MASK]);
      }
    }

    // write sign
    if constexpr (trait::sint_<decltype(val)>) {
      if (val < 0) this->push('-');
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
  const auto type = f.type();
  const auto upcase = type == 'X' || type == 'P';
  const auto uval = __builtin_bit_cast(usize, val);

  char buf[16];
  const auto s = RevBuf{buf}.write_ptr(uval, upcase);
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

void Formatter::pad_num(bool is_neg, Str body) {
  const auto sign = _spec.sign(is_neg);
  const auto prefix = _spec.prefix();
  if (body._len >= _spec._width) {
    this->write_str(sign);
    this->write_str(prefix);
    this->write_str(body);
    return;
  }

  const auto npad = _spec._width - body._len;
  const auto fill = _spec.fill(_spec._prefix ? '0' : ' ');
  const auto align = _spec.align(fill == '0' ? '=' : '>');
  switch (align) {
    default:
    case '>':
      this->write_chars(fill, npad);
      this->write_str(sign);
      this->write_str(prefix);
      this->write_str(body);
      break;
    case '<':
      this->write_str(sign);
      this->write_str(prefix);
      this->write_str(body);
      this->write_chars(fill, npad);
      break;
    case '=':
      this->write_str(sign);
      this->write_str(prefix);
      this->write_chars(fill, npad);
      this->write_str(body);
      break;
    case '^':
      this->write_chars(fill, (npad + 0) / 2);
      this->write_str(sign);
      this->write_str(prefix);
      this->write_str(body);
      this->write_chars(fill, (npad + 1) / 2);
  }
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

auto Formatter::debug_tuple(Str name) -> DebugTuple {
  this->write_str(name);
  return DebugTuple{*this};
}

auto Formatter::debug_struct(Str name) -> DebugStruct {
  this->write_str(name);
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
