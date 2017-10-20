#include "config.h"

namespace ustd::fmt
{

#pragma region fmt: str
static void fmt_str_impl(const Formatter& spec, String& outbuf, const char text[], u32 text_len) {
    let num_width = i32(spec.width);
    let num_chars = i32(text_len);

    let p = outbuf.data() + outbuf.len();
    if (spec.width <= num_chars) {
        outbuf._size += num_chars;

        for (i32 i = 0; i < num_chars; ++i) {
            p[i] = text[i];
        }
    }
    else {
        outbuf._size += spec.width;

        let num_fills  = num_width - num_chars;
        if (spec.align == '<') {
            for (i32 i = 0; i < num_chars; ++i) {
                p[i] = text[i];
            }
            for (i32 i = num_chars; i < num_width; ++i) {
                p[i] = spec.fill;
            }
        }
        else if (spec.align == '^') {
            for (i32 i = 0; i < num_fills / 2; ++i) {
                p[i] = spec.fill;
            }
            for (i32 i = num_fills / 2; i < num_fills / 2 + num_chars; ++i) {
                p[i] = text[i - num_fills / 2];
            }
            for (i32 i = num_fills / 2 + num_chars; i < num_width; ++i) {
                p[i] = spec.fill;
            }
        }
        else {
            for (i32 i = 0; i < num_fills; ++i) {
                p[i] = spec.fill;
            }
            for (i32 i = num_fills; i < num_width; ++i) {
                p[i] = text[i-num_fills];
            }
        }
        
    }
}

template<u32 N>
static fn fmt_str_impl(const Formatter& spec, String& outbuf, const char (&text)[N]) -> void {
    fmt_str_impl(spec, outbuf, text, N-1);
}

#pragma endregion

#pragma region fmt: int
inline fn count_digits(u32 n) -> u32{
    static const u32 powers[] = {
        0,
        1,
        10,
        100,
        1000,
        10000,
        100000,
        1000000,
        10000000,
        100000000,
        1000000000
    };

    let t = (32 - __builtin_clz(n | 1)) * 1233 >> 12;
    return u32(t) - (n < powers[t]) + 1;
}

static fn count_digits(u64 n) -> u32{
    static const u64 powers[] = {
        0,
        1,
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
        1000000000000000000,
        10000000000000000000ull
    };

    let t = (64 - __builtin_clzll(n|1))*1233 >> 12;
    return u32(t) - (n < powers[t]) + 1;
}

static fn count_digits(u16 n) -> u32 {
    return count_digits(u32(n));
}

static fn count_digits(u8 n) -> u32 {
    return count_digits(u32(n));
}

static fn fmt_int_box(const Formatter& spec, String& outbuf, const char prefix[], u32 num_prefix, u32 num_digits) -> char* {
    let s = outbuf.data() + outbuf.len();

    // check: spec.width is small
    if (spec.width < num_digits + num_prefix) {
        goto END;
    }

    outbuf._size += u32(spec.width);
    if (spec.align == '<') {
        // ....####~~~~....
        //     ^   ^   ^
        //     s   p   e
        let e = s + spec.width;
        for (u32 i = 0; i < num_prefix; ++i) {
            s[i] = prefix[i];
        }
        let p = s + num_prefix + num_digits;

        for (mut f = p; f < e; ++f) {
            *f = spec.fill;
        }
        return p;
    }
    else if (spec.align == '^') {
        // ....~~~####~~~....
        //     ^  ^   ^  ^
        //     s  q   p  e
        let e = s + spec.width;
        let q = s + (spec.width - num_prefix - num_digits)/2;
        let p = q + num_prefix + num_digits;

        for (u32 i = 0; i < num_prefix; ++i) {
            q[i] = prefix[i];
        }

        for (mut f = s; f < q; ++f) {
            *f = spec.fill;
        }

        for (mut f = p; f < e; ++f) {
            *f = spec.fill;
        }
        return p;
    }
    else {  // >
        // ....~~~~####....
        //     ^   ^   ^
        //     s   q   p
        let q = s + spec.width - num_prefix - num_digits;
        let p = s + spec.width;

        for (mut f = s; f < q; ++f) {
            *f = spec.fill;
        }

        for (u32 i = 0; i < num_prefix; ++i) {
            q[i] = prefix[i];
        }
        return p;
    }

END:
    outbuf._size += num_prefix + num_digits;
    let p = s + num_prefix + num_digits;

    for (u32 i = 0; i < num_prefix; ++i) {
        s[i] = prefix[i];
    }
    return p;
}

static fn fmt_int_body(char* p, u64 value) -> void {
    static const char digits[] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";

    while (value >= 100) {
      let index = u32((value % 100) * 2);
      value /= 100;
      *--p = digits[index + 1];
      *--p = digits[index];
    }

    if (value < 10) {
        *--p = char('0' + value);
        return;
    }

    let index = u32(value * 2);
    *--p = digits[index + 1];
    *--p = digits[index];
}

template<typename Tint>
static fn fmt_int_impl(const Formatter& spec, String& outbuf, Tint value) -> void {
    let abs_value = to_uint(value > 0 ? Tint(value) : Tint(0 - value));

    char prefix[4];
    mut num_prefix = 0u;

    if (value < 0) {
        prefix[0] = '-';
        ++num_prefix;
    }
    else if (spec.sign=='+') {
        prefix[0] = '+';
        ++num_prefix;
    }
    else if (spec.sign == ' ') {
        prefix[0] = ' ';
        ++num_prefix;
    }

    switch (spec.type) {
    case 0: case 'd': {
        let num_digits = count_digits(abs_value);
        let p          = fmt_int_box(spec, outbuf, prefix, num_prefix, num_digits);
        fmt_int_body(p, abs_value);
        break;
    }
    case 'x': case 'X': {
        prefix[num_prefix++] = '0';
        prefix[num_prefix++] = spec.type;

        mut num_digits = 0u;
        {
            mut n = abs_value;
            do {
                ++num_digits;
            } while ((n >>= 4) != 0);
        }

        let digits = spec.type == 'x' ? "0123456789abcdef" : "0123456789ABCDEF";

        mut p = fmt_int_box(spec, outbuf, prefix, u32(num_prefix), num_digits);
        mut n = abs_value;
        do {
            *--p = digits[n & 0xf];
        } while ((n >>= 4) != 0);
        break;
    }
    case 'o': {
        prefix[num_prefix++] = '0';

        mut num_digits = 0u;
        {
            mut n = abs_value;
            do {

                ++num_digits;
            } while ((n >>= 3) != 0);
        }

        mut p = fmt_int_box(spec, outbuf, prefix, num_prefix, num_digits);
        mut n = abs_value;
        do {
            *--p = char('0' + (n & 7));
        } while ((n >>= 3) != 0);
        break;
    }
    case 'b': case 'B': {
        prefix[num_prefix++] = '0';
        prefix[num_prefix++] = spec.type;

        mut num_digits = 0u;
        {
            mut n = abs_value;
            do {
                ++num_digits;
            } while ((n >>= 1) != 0);
        }

        mut p = fmt_int_box(spec, outbuf, prefix, num_prefix, num_digits);
        mut n = abs_value;
        do {
            *--p = char('0' + (n & 1));
        } while ((n >>= 1) != 0);
        break;
    }
    case 'c' : {
        mut p = fmt_int_box(spec, outbuf, prefix, num_prefix, 1);
        *--p = char(value);
        break;
    }
    default:
        break;
    }
}

#pragma endregion

#pragma region fmt: float
template<typename Tfloat>
static fn fmt_float_impl(const Formatter& spec, String& outbuf, Tfloat value) -> void {
    if (isinf(value)) {
        fmt_str_impl(spec, outbuf, "inf");
        return;
    }
    if (isnan(value)) {
        fmt_str_impl(spec, outbuf, "nan");
        return;
    }

    let abs_value = value >= 0 ? value : -value;

    char buff[256];

    let p = spec.width == 0 ? (outbuf.data() + outbuf.len()) : buff;
    mut num_digits = 0;

    if (value < 0) {
        p[num_digits++] = '-';
    }
    else if (spec.sign == '+') {
        p[num_digits++] = '+';
    }
    else if (spec.sign == ' ') {
        p[num_digits++] = ' ';
    }

    if (spec.prec == 0) { // default
        num_digits += spec.type == 'e'
            ? ::snprintf(p, 64, "%e", abs_value) : spec.type == 'g'
            ? ::snprintf(p, 64, "%g", abs_value)
            : ::snprintf(p, 64, "%f", abs_value);
    }
    else {
        num_digits += spec.type == 'e'
            ? ::snprintf(p, 64, "%.*e", spec.prec, abs_value) : spec.type == 'g'
            ? ::snprintf(p, 64, "%.*g", spec.prec, abs_value)
            : ::snprintf(p, 64, "%.*f", spec.prec, abs_value);
    }

    if (p == buff) {
        fmt_str_impl(spec, outbuf, buff, u32(num_digits));
    }
    else {
        outbuf._size += num_digits;
    }
}
#pragma endregion

#pragma region fmt: parse
fn _sfmt_parse(String& outbuf, str& fmtstrs, str *fmtstr, i32* idx) -> bool {
    // ......{........}....
    //       ^        ^    ^
    //       p        q    e

    let s = fmtstrs.data();
    let e = s + fmtstrs.len();
    mut p = s;

    // find: p
    while (p < e) {
        let c = *p;
        if (c == '{') {
            if (p + 1 < e && *(p + 1) == '{') {
                outbuf.push('{');
                ++p;
            }
            else {
                break;
            }
        }
        else if (c == '}') {
            if (p + 1 < e && *(p + 1) == '}') {
                outbuf.push('}');
                ++p;
            }
        }
        else {
            outbuf.push(c);
        }
        ++p;
    }

    if (p == e) {
        return false;
    }

    // find: q
    mut q = p + 1;
    while(q < e) {
        let c = *q;
        if (c=='}') {
            break;
        }
        q++;
    }

    if (q == e) {
        return false;
    }

    // return
    fmtstrs = str{ q + 1, u32(e - q - 1) };
    *fmtstr = str{ p + 1, u32(q - p - 1) };
    *idx    += 1;

    return true;
}
#pragma endregion

#pragma region Formatter
// [[fill]align][sign][width]['.'precision][type]
fn Formatter::from_str(str text) -> Formatter {
    mut spec = Formatter{};

    if (text.len() ==0) {
        return spec;
    }

    mut p = text.data();

    // [[fill]align]
    if (p[1] == '>' || p[1] == '<' || p[1] == '^') {
        spec.fill  = p[0];
        spec.align = p[1];
        p += 2;
    }
    else if(p[0] == '>' || p[0] == '<' || p[0] == '^') {
        spec.fill  = ' ';
        spec.align = p[0];
        p += 1;
    }

    // [sign]
    if (p[0] == '+' || p[0] == '-' || p[0] == ' ') {
        spec.sign = p[0];
        p += 1;
    }

    // [width]
    spec.width = 0;
    while('0' <= p[0] && p[0] <= '9') {
        spec.width = u8(spec.width*10 + (p[0] - '0'));
        p += 1;
    }

    // [prec]
    if (p[0] == '.') {
        p += 1;

        spec.prec = 0;
        while('0' <= p[0] && p[0] <= '9') {
            spec.prec = u8(spec.prec *10 + (p[0] - '0'));
            p += 1;
        }
    }

    // [type]
    if (p[0]!='}') {
        spec.type = p[0];
    }

    return spec;
}

fn Formatter::sfmt_val(String& outbuf, i8   val) const -> void { fmt_int_impl(*this, outbuf, val); }
fn Formatter::sfmt_val(String& outbuf, u8   val) const -> void { fmt_int_impl(*this, outbuf, val); }
fn Formatter::sfmt_val(String& outbuf, i16  val) const -> void { fmt_int_impl(*this, outbuf, val); }
fn Formatter::sfmt_val(String& outbuf, u16  val) const -> void { fmt_int_impl(*this, outbuf, val); }
fn Formatter::sfmt_val(String& outbuf, i32  val) const -> void { fmt_int_impl(*this, outbuf, val); }
fn Formatter::sfmt_val(String& outbuf, u32  val) const -> void { fmt_int_impl(*this, outbuf, val); }
fn Formatter::sfmt_val(String& outbuf, i64  val) const -> void { fmt_int_impl(*this, outbuf, val); }
fn Formatter::sfmt_val(String& outbuf, u64  val) const -> void { fmt_int_impl(*this, outbuf, val); }

fn Formatter::sfmt_val(String& outbuf, f32  val) const -> void { fmt_float_impl(*this, outbuf, val); }
fn Formatter::sfmt_val(String& outbuf, f64  val) const -> void { fmt_float_impl(*this, outbuf, val); }

fn Formatter::sfmt_val(String& outbuf, str  val) const -> void {
    if (this->type != 's') {
        if (this->width < 2) {
            fmt_str_impl(*this, outbuf, val.data(), val.len());
        }
        else {
            mut tmp = *this;
            tmp.width -= 2;
            fmt_str_impl(tmp, outbuf, val.data(), val.len());
        }
        outbuf.push('"');
    }
    else {
        fmt_str_impl(*this, outbuf, val.data(), val.len());
    }
}

fn Formatter::sfmt_val(String& outbuf, bool val) const -> void {
    val
        ? fmt_str_impl(*this, outbuf, "true")
        : fmt_str_impl(*this, outbuf, "false");
}

#pragma endregion

#pragma region unittest

[unittest(Formatter::sfmt_val<i32>)]
{
    let num = 12345;
    println("    fmt |{^20s}|{^20s}|{^20s}|{^20s}|", ">", "<", "^", "~");
    println("    dec |{>20}|{<20}|{^20}|{20}|",      num, num, num, num);
    println("    bin |{>20b}|{<20b}|{^20b}|{20b}|",  num, num, num, num);
    println("    oct |{>20o}|{<20o}|{^20o}|{20o}|",  num, num, num, num);
    println("    hex |{>20x}|{<20x}|{^20x}|{20x}|",  num, num, num, num);
    println("    HEX |{>20X}|{<20X}|{^20X}|{20X}|",  num, num, num, num);
};

[unittest(Formatter::sfmt_val<f32>)]
{
    let num = 123.45;
    println("    fmt  |{^20s}|{^20s}|{^20s}|{^20s}|",        ">", "<", "^", "~");
    println("    20f  |{>20f}|{<20f}|{^20f}|{20f}|",         num, num, num, num);
    println("    20e  |{>20e}|{<20e}|{^20e}|{20e}|",         num, num, num, num);
    println("    20.2e|{>20.2e}|{<20.2e}|{^20.2e}|{20.2e}|", num, num, num, num);
    println("    20.2f|{>20.2f}|{<20.2f}|{^20.2f}|{20.2f}|", num, num, num, num);
};

[unittest(Formatter::sfmt_tuple)]
{
    let t = tuple("a", 1, "b", 2);
    println("    tuple| {}", t);
};

#pragma endregion

}
