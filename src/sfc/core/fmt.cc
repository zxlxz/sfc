#include "sfc/core/fmt.h"
#include "sfc/core/num.h"

namespace sfc::fmt {

namespace detail {

constexpr u32 MAX_PRECISION = 20U;

struct FixFloat {
  u64 _int;  // int part
  u64 _flt;  // flt part
};

struct ExpFloat {
  u64 _base;  // fixed point value
  i32 _iexp;  // exponent, the actual value is _val * 10^(-_exp)
};

auto exp10(int n) -> f64 {
  constexpr u32 TBL_LEN = 32U;
  constexpr f64 EXP_TBL[] = {
      1e0,  1e1,  1e2,  1e3,  1e4,  1e5,  1e6,  1e7,  1e8,  1e9,  1e10, 1e11, 1e12, 1e13, 1e14, 1e15,
      1e16, 1e17, 1e18, 1e19, 1e20, 1e21, 1e22, 1e23, 1e24, 1e25, 1e26, 1e27, 1e28, 1e29, 1e30, 1e31,
  };

  auto u = n < 0 ? u32(-n) : u32(n);
  if (u == 0) {
    return 1.0;
  }

  auto x = 1.0;
  for (; u >= TBL_LEN; u -= TBL_LEN) {
    x *= 1e32;
  }
  x *= EXP_TBL[u];

  if (n < 0) {
    x = 1.0 / x;
  }
  return x;
}

auto frexp10(f64 uflt, int* exp) {
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

auto prec_ratio(u32 precision) -> u64 {
  constexpr u64 EXP_10 = u64(1e10);   // 1e10
  constexpr u64 EXP_MAX = u64(1e19);  // 1e19
  constexpr u64 EXP_TBL[10] = {1U, 10U, 100U, 1000U, 10000U, 100000U, 1000000U, 10000000U, 100000000U, 1000000000U};

  if (precision < 10) return EXP_TBL[precision];
  if (precision < 20) return EXP_10 * EXP_TBL[precision - 10];
  return EXP_MAX;
}

auto build_fix(f64 uflt, u32 precision) -> FixFloat {
  if (precision >= MAX_PRECISION) {
    precision = MAX_PRECISION - 1;
  }

  // 1000...
  const auto prec_ratio = detail::prec_ratio(precision);

  // extract integer part
  auto int_part = u64(uflt);
  auto flt_part = u64(__builtin_round((uflt - f64(int_part)) * f64(prec_ratio)));
  if (flt_part >= prec_ratio) {
    flt_part -= prec_ratio;
    int_part += 1;
  }

  // add 1000... to flt_part
  // to ensure it has leading zeros when converted to string
  flt_part += prec_ratio;

  return FixFloat{int_part, flt_part};
}

auto build_exp(f64 uflt, u32 precision) -> ExpFloat {
  const auto prec_ratio = detail::prec_ratio(precision);

  auto exp_cnt = 0;
  auto flt_val = frexp10(uflt, &exp_cnt);

  auto int_part = u64(__builtin_round(flt_val * f64(prec_ratio)));
  if (int_part / 10 >= prec_ratio) {
    int_part /= 10;
    exp_cnt += 1;
  }

  return ExpFloat{u64(int_part), exp_cnt};
}

struct RevBuf {
  static constexpr usize BUF_SIZE = 128;

  char _buf[BUF_SIZE];
  usize _pos = BUF_SIZE;

 public:
  auto as_str() const -> Str {
    return Str{_buf + _pos, BUF_SIZE - _pos};
  }

  auto write_u64(u64 uval, char type = 0) noexcept -> Str {
    switch (type) {
      default:  this->push_dec(uval); break;
      case 'B':
      case 'b': this->push_bin<2>(uval); break;
      case 'O':
      case 'o': this->push_bin<8>(uval); break;
      case 'X':
      case 'x': this->push_bin<16>(uval, type); break;
    }
    return this->as_str();
  }

  auto write_ptr(usize uval, char type) noexcept -> Str {
    constexpr u32 MIN_LEN = 12;
    const auto upcase = 'A' <= type && type <= 'Z';

    this->push_bin<16>(uval, type);
    while (BUF_SIZE - _pos < MIN_LEN) {
      this->push('0');
    }
    this->push(upcase ? 'X' : 'x');
    this->push('0');
    return this->as_str();
  }

  auto write_flt(f64 val, u32 precision, char type) -> Str {
    constexpr auto MAX_UVAL = f64(num::Int<u64>::MAX);

    if (__builtin_isnan(val)) {
      return "NaN";
    }
    if (__builtin_isinf(val)) {
      return val > 0 ? Str{"inf"} : Str{"-inf"};
    }

    if (precision > MAX_PRECISION) {
      precision = MAX_PRECISION;
    }

    const auto uval = __builtin_fabs(val);
    const auto upcase = chr::is_upcase(type);
    const auto use_exp = type == 'e' || type == 'E' || uval >= MAX_UVAL;
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
  void push_bin(u64 val, char type = 0) noexcept {
    static_assert((RADIX & (RADIX - 1)) == 0, "radix must be power of 2");
    constexpr auto MASK = RADIX - 1;

    const auto digits_upper = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const auto digits_lower = "0123456789abcdefghijklmnopqrstuvwxyz";

    // write digits
    const auto upcase = 'A' <= type && type <= 'Z';
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

}  // namespace detail

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
  if (_alt != '#') {
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

auto Buf::as_str() const -> Str {
  const auto len = cmp::min(_len, _buf._len);
  return Str{_buf._ptr, len};
}

void Buf::truncate(usize new_len) {
  _len = cmp::min(new_len, _buf._len);
}

void Buf::write_str(Str s) noexcept {
  if (_len >= _buf._len || s._len == 0) {
    return;
  }
  const auto n = cmp::min(s._len, _buf._len - _len);
  ptr::copy_nonoverlapping(s._ptr, _buf._ptr + _len, n);
  _len += n;
}

void Debug::fmt(bool val, Formatter& f) {
  f.write_str(val ? Str{"true"} : Str{"false"});
}

void Debug::fmt(char val, Formatter& f) {
  f.write_char(val);
}

void Debug::fmt(char16_t val, Formatter& f) {
  u8 buf[4] = {};

  const auto v = chr::encode_utf8(buf, val);
  f.write_str(Str::from_utf8(v));
}

void Debug::fmt(char32_t val, Formatter& f) {
  u8 buf[4] = {};

  const auto v = chr::encode_utf8(buf, val);
  f.write_str(Str::from_utf8(v));
}

void Debug::fmt(unsigned int val, Formatter& f) {
  auto buf = detail::RevBuf{};
  const auto s = buf.write_u64(val, f.type());
  f.pad_num(false, s);
}

void Debug::fmt(unsigned long val, Formatter& f) {
  auto buf = detail::RevBuf{};
  const auto s = buf.write_u64(val, f.type());
  f.pad_num(false, s);
}

void Debug::fmt(unsigned long long val, Formatter& f) {
  auto buf = detail::RevBuf{};
  const auto s = buf.write_u64(val, f.type());
  f.pad_num(false, s);
}

void Debug::fmt(int val, Formatter& f) {
  const auto uval = num::unsigned_abs(val);

  auto buf = detail::RevBuf{};
  const auto s = buf.write_u64(uval, f.type());
  f.pad_num(val < 0, s);
}

void Debug::fmt(long val, Formatter& f) {
  const auto uval = num::unsigned_abs(val);

  auto buf = detail::RevBuf{};
  const auto s = buf.write_u64(uval, f.type());
  f.pad_num(val < 0, s);
}

void Debug::fmt(long long val, Formatter& f) {
  const auto uval = num::unsigned_abs(val);

  auto buf = detail::RevBuf{};
  const auto s = buf.write_u64(uval, f.type());
  f.pad_num(val < 0, s);
}

void Debug::fmt(float val, Formatter& f) {
  constexpr u32 DEFAULT_PRECISION = 4U;
  const auto prec = f.precision().unwrap_or(DEFAULT_PRECISION);

  const auto uval = val < 0 ? -val : val;
  auto buf = detail::RevBuf{};
  const auto s = buf.write_flt(uval, prec, f.type());
  f.pad_num(val < 0, s);
}

void Debug::fmt(double val, Formatter& f) {
  constexpr u32 DEFAULT_PRECISION = 6U;
  const auto prec = f.precision().unwrap_or(DEFAULT_PRECISION);

  const auto uval = val < 0 ? -val : val;

  auto buf = detail::RevBuf{};
  const auto s = buf.write_flt(uval, prec, f.type());
  f.pad_num(val < 0, s);
}

void Debug::fmt([[maybe_unused]] decltype(nullptr) val, Formatter& f) {
  f.write_str("nullptr");
}

void Debug::fmt(const void* val, Formatter& f) {
  const auto type = f.type();
  const auto uval = __builtin_bit_cast(usize, val);

  auto buf = detail::RevBuf{};
  const auto s = buf.write_ptr(uval, type);
  f.write_str(s);
}

void Debug::fmt(const char* val, Formatter& f) {
  const auto s = Str::from_cstr(val);
  s.fmt(f);
}

auto Formatter::spec() const -> Spec {
  return _spec;
}

auto Formatter::type() const -> char {
  return _spec._type;
}

auto Formatter::width() const -> Option<u32> {
  if (!_spec._width) return {};
  return _spec._width;
}

auto Formatter::precision() const -> Option<u32> {
  if (!_spec._point) return {};
  return _spec._precision;
}

auto Formatter::depth() const -> u32 {
  return _depth;
}

auto Formatter::write_str(Str s) -> Formatter& {
  if (s._len != 0) {
    _out.write_str(s);
  }
  return *this;
}

auto Formatter::write_char(char c) -> Formatter& {
  _out.write_str({&c, 1});
  return *this;
}

auto Formatter::write_pad(char c, usize n) -> Formatter& {
  constexpr u32 BUF_LEN = 16;
  char buf[BUF_LEN] = {c, c, c, c, c, c, c, c, c, c, c, c, c, c, c, c};

  for (auto rem = n; rem > 0;) {
    const auto chunk = rem < BUF_LEN ? rem : BUF_LEN;
    this->write_str({buf, chunk});
    rem -= chunk;
  }
  return *this;
}

void Formatter::pad(Str s) {
  const auto pad_len = _spec._width < s._len ? 0 : _spec._width - s._len;
  const auto pad_chr = _spec.fill();

  switch (_spec._align) {
    default:
    case '<':
      this->write_str(s);
      this->write_pad(pad_chr, pad_len);
      break;
    case '>':
      this->write_pad(pad_chr, pad_len);
      this->write_str(s);
      break;
    case '=':
    case '^':
      this->write_pad(pad_chr, (pad_len + 0) / 2);
      this->write_str(s);
      this->write_pad(pad_chr, (pad_len + 1) / 2);
      break;
  }
}

void Formatter::pad_num(bool is_neg, Str body) {
  const auto pad_len = _spec._width < body._len ? 0 : _spec._width - body._len;
  const auto pad_chr = _spec.fill(_spec._alt ? '0' : ' ');

  const auto sign = _spec.sign(is_neg);
  const auto prefix = _spec.prefix();
  const auto align = _spec.align(pad_chr == '0' ? '=' : '>');

  switch (align) {
    default:
    case '>':
      this->write_pad(pad_chr, pad_len);
      this->write_str(sign);
      this->write_str(prefix);
      this->write_str(body);
      break;
    case '<':
      this->write_str(sign);
      this->write_str(prefix);
      this->write_str(body);
      this->write_pad(pad_chr, pad_len);
      break;
    case '=':
      this->write_str(sign);
      this->write_str(prefix);
      this->write_pad(pad_chr, pad_len);
      this->write_str(body);
      break;
    case '^':
      this->write_pad(pad_chr, (pad_len + 0) / 2);
      this->write_str(sign);
      this->write_str(prefix);
      this->write_str(body);
      this->write_pad(pad_chr, (pad_len + 1) / 2);
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

auto Formatter::Block::item(Str begin) -> Formatter& {
  const auto is_pretty = _fmt._spec._alt == '#';

  if (_fields == 0) {
    _fmt.write_str(begin);

    if (is_pretty) {
      _fmt._depth += 1;
    }
  }

  if (is_pretty) {
    if (_fields == 0) {
      _fmt.write_str("\n");
    } else {
      _fmt.write_str(",\n");
    }
    this->indent();
  } else {
    if (_fields != 0) {
      _fmt.write_str(", ");
    }
  }

  _fields += 1;

  return _fmt;
}

void Formatter::Block::indent() {
  constexpr u32 TAB_SIZE = 2U;
  const auto indent_len = _fmt._depth * TAB_SIZE;
  _fmt.write_pad(' ', indent_len);
}

void Formatter::Block::finish(Str begin, Str end) {
  const auto is_pretty = _fmt._spec._alt == '#';
  const auto has_fields = _fields > 0;

  if (has_fields) {
    if (is_pretty) {
      _fmt._depth -= 1;
      _fmt.write_str("\n");
      this->indent();
    }
  } else {
    _fmt.write_str(begin);
  }

  _fmt.write_str(end);
}

auto DebugMap::key(Str key) -> DebugMap& {
  auto& fmter = _blk.item("{");
  fmter.write_str("\"");
  fmter.write_str(key);
  fmter.write_str("\": ");
  return *this;
}

auto DebugStruct::key(Str key) -> DebugStruct& {
  auto& fmter = _blk.item("{");
  fmter.write_str(key);
  fmter.write_str(": ");
  return *this;
}

}  // namespace sfc::fmt
