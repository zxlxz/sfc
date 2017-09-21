#include <nms/test.h>

namespace nms
{

template<class Tnum>
static int _parse_number(StrView str, StrView fmt, Tnum& num) {
    auto ptr = str.data();
    auto cnt = str.count();
    auto pos = 0u;

    // [c] ?
    if (fmt.count() > 0 && fmt[0u] == 'c') {
        num = Tnum(ptr[pos++]);
        return int(pos);
    }

    Tnum val = 0;

    // ignore space
    while (pos < cnt && isblank(ptr[pos])) {
        pos++;
    }
    if (pos == cnt) {
        return 0;
    }

    // [+-]?
    const auto sign = ptr[pos];
    switch (sign) {
        case '+':
        case '-':
            ++pos;
            break;
        default:
            break;
    }

    if ($is<$uint, Tnum> && sign == '-') {
        goto END;
    }

    if ($is<$int, Tnum> && fmt.count() > 0) {
        // oct
        if (fmt[0] == 'o' || fmt[0] == 'O') {
            constexpr static const Tnum radix = 8;
            while (pos < cnt) {
                auto c = ptr[pos++];
                if (c >= '0' && c <= '7') {
                    val *= radix;
                    val += Tnum(c - '0');
                }
                else {
                    break;
                }
            }
            goto END;
        }

        // hex
        else if (fmt[0] == 'x' || fmt[0] == 'X') {
            constexpr static const Tnum radix = 16;
            while (pos < cnt) {
                auto c = ptr[pos++];
                if (c >= '0' && c <= '9') {
                    val *= radix;
                    val += Tnum(c - '0');
                }
                else if (c >= 'a' && c <= 'f') {
                    val *= radix;
                    val += Tnum(10);
                    val += Tnum(c - 'a');
                }
                else if (c >= 'A' && c <= 'F') {
                    val *= radix;
                    val += Tnum(10);
                    val += Tnum(c - 'A');
                }
                else {
                    break;
                }
            }
            goto END;
        }
    }

    // body
    while(pos < cnt) {
        auto c = ptr[pos];
        if (c >= '0' && c <= '9') {
            val *= 10;
            val += Tnum(c - '0');
            ++pos;
        }
        else {
            break;
        }
    }

    // float?
    if ($is<$float, Tnum>) {
        // . ?
        if (ptr[pos] == '.') {
            pos++;

            Tnum scale = 1;
            while (pos < cnt) {
                scale /= Tnum(10);

                auto c = ptr[pos];
                if (c >= '0' && c <= '9') {
                    val += Tnum(c - '0') * scale;
                    ++pos;
                }
                else {
                    break;
                }
            }
        }

        // eE
        switch (ptr[pos]) {
            case 'e':
                pos++;
                break;
            case 'E':
                pos++;
                break;
            default:
                goto END;
        }

        auto esign = ptr[pos];
        switch (esign) {
            case '+':
            case '-':
                ++pos;
                break;
            case '0': case '1': case '2': case '3': case '4': case '5' : case '6': case '7': case '8': case '9':
                break;
            default:
                --pos;
                goto END;
        }

        int exp = 0;
        while (pos < cnt) {
            auto c = ptr[pos];
            if (c >= '0' && c <= '9') {
                exp = exp * 10 + int(c - '0');
                ++pos;
            }
            else {
                break;
            }
        }

        if (esign == '-') {
            exp = -exp;
        }

        if (exp > 0) {
            for (int i = 0; i < exp; ++i) {
                val *= Tnum(10);
            }
        }
        else if (exp < 0) {
            for (int i = 0; i < exp; ++i) {
                val /= Tnum(10);
            }
        }
    }

END:
    if (sign == '-') {
        num = Tnum(Tnum(0) - val);
    }
    else {
        num = val;
    }
    return int(pos);
}

NMS_API int _parse(StrView str, StrView fmt, u8&  val) { return _parse_number(str, fmt, val); }
NMS_API int _parse(StrView str, StrView fmt, i8&  val) { return _parse_number(str, fmt, val); }
NMS_API int _parse(StrView str, StrView fmt, u16& val) { return _parse_number(str, fmt, val); }
NMS_API int _parse(StrView str, StrView fmt, i16& val) { return _parse_number(str, fmt, val); }
NMS_API int _parse(StrView str, StrView fmt, u32& val) { return _parse_number(str, fmt, val); }
NMS_API int _parse(StrView str, StrView fmt, i32& val) { return _parse_number(str, fmt, val); }
NMS_API int _parse(StrView str, StrView fmt, u64& val) { return _parse_number(str, fmt, val); }
NMS_API int _parse(StrView str, StrView fmt, i64& val) { return _parse_number(str, fmt, val); }
NMS_API int _parse(StrView str, StrView fmt, f32& val) { return _parse_number(str, fmt, val); }
NMS_API int _parse(StrView str, StrView fmt, f64& val) { return _parse_number(str, fmt, val); }

}