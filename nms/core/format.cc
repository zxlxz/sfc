#include <nms/core/format.h>

namespace nms
{

/* === format spec === */
struct FmtSpec
{
    u8      align   = '\0';
    u8      sign    = '\0';
    u8      type    = '\0';
    u32     width   = 0;
    u32     prec    = 0;

    FmtSpec(u32 width, u32 prec)
        : width(width), prec(prec)
    {}

    explicit FmtSpec(StrView fmt) {
        if (fmt.count() == 0) return;

        const auto n = fmt.count();
        auto       i = 0u;

        // parse: align
        if (i < n && (fmt[i] == '<' || fmt[i] == '>' || fmt[i] == '^')) {
            align = fmt[i++];
        }

        // parse: sign
        if (i < n && (fmt[i] == '+' || fmt[i] == '-')) {
            sign = fmt[i++];
        }

        // parse: width
        while (i < n && ('0' <= fmt[i] && fmt[i] <= '9')) {
            width = width * 10 + (fmt[i++] - '0');
        }

        // parse: spec
        if (i < n && (fmt[i] == '.')) {
            ++i;
            while (i < n && ('0' <= fmt[i] && fmt[i] <= '9')) {
                prec = prec * 10 + (fmt[i++] - '0');
            }
        }

        // parse: type
        if (i < n) {
            type = fmt[i];
        }
    }
};

// [<>=][width]
static void _formatStr(String& buf, StrView sfmt, StrView val) {
    const auto cnt = val.count();

    if (sfmt.count()==0) {
        buf += val;
    }
    else {
        FmtSpec fmt(sfmt);
        if (fmt.width <= cnt) {
            buf += val;
        }
        else {
            switch (fmt.align) {
            case '<': default:
                buf += val;
                buf.appends(fmt.width - cnt, ' ');
                break;
            case '>':
                buf.appends(fmt.width - cnt, ' ');
                buf += val;
                break;
            case '^':
                buf.appends((fmt.width - cnt + 0) / 2, ' ');
                buf += val;
                buf.appends((fmt.width - cnt + 1) / 2, ' ');
                break;
            }
        }
    }
}

// [align:<>=][width:number].[prec:number]s
template<class T>
static void _formatInt(String& buf, StrView sfmt, T val, StrView type) {
    char str[256];

    if (sfmt.count()==0) {
        const auto n = snprintf(str, sizeof(str), type.data(), val);
        const auto s = StrView(str, { u32(n) });
        buf += s;
    }
    else {
        const FmtSpec fmt(sfmt);

        const auto  uval = val < 0 ? 0 - val : val;
        auto        len = 0u;

        switch (fmt.type) {
        case 'c':
            str[0] = char(val);
            len = 1;
            break;

        case ' ':
            len = min(u32(val), 256u);
            memset(str, ' ', len);
            break;

        default:
            if (val < 0) {
                str[0] = '-'; ++len;
            }
            else {
                if (fmt.sign == '+') { str[0] = '+'; ++len; }
                if (fmt.sign == '-') { str[0] = ' '; ++len; }
            }
            len += snprintf(str + len, sizeof(str) - len, type.data(), uval);
            break;
        }

        if (fmt.width <= len) {
            buf += StrView{ str, {len } };
        }
        else {
            switch (fmt.align) {
            case '<':
                buf+= StrView{ str, {len } };
                buf.appends(fmt.width - len, ' ');
                break;
            case '>': default:
                buf.appends(fmt.width - len, ' ');
                buf+= StrView{ str, {len} };
                break;
            case '^':
                buf.appends((fmt.width - len + 0) / 2, ' ');
                buf+= StrView{ str, {len } };
                buf.appends((fmt.width - len + 1) / 2, ' ');
                break;
            }
        }
    }
}

// [align:<>=][sign:+-][width:number].[prec:number]
template<class T>
static void _formatFlt(String& buf, StrView sfmt, T val) {
    const auto fmt = sfmt.count() > 0 ? FmtSpec(sfmt): $is<float, T> ? FmtSpec(6, 3) : FmtSpec(9, 6);

    char tmp[256];
    auto uval = val < 0 ? -val : val;
    auto ptr = tmp + 1;
    auto len = u32(snprintf(ptr, sizeof(tmp) - 1, "%.*f", fmt.prec, uval));

    if (val < 0) {
        tmp[0] = '-'; ++len; --ptr;
    }
    else {
        if (fmt.sign == '+') { tmp[0] = '+'; ++len; --ptr; }
        if (fmt.sign == '-') { tmp[0] = ' '; ++len; --ptr; }
    }

    if (len >= fmt.width) {
        buf += StrView{ ptr, {len } };
    }
    else {
        switch (fmt.align) {
        case '<':
            buf += StrView{ ptr, {len } };
            buf.appends(fmt.width - len, ' ');
            break;
        case '>': default:
            buf.appends(fmt.width - len, ' ');
            buf += StrView{ ptr, {len } };
            break;
        case '^':
            buf.appends((fmt.width - len + 0) / 2, ' ');
            buf += StrView{ ptr, {len } };
            buf.appends((fmt.width - len + 1) / 2, ' ');
            break;
        }
    }
}

NMS_API void formatImpl(String& buf, StrView fmt, StrView val) { _formatStr(buf, fmt, val);              }
NMS_API void formatImpl(String& buf, StrView fmt, i8      val) { _formatInt(buf, fmt, val,      "%d");   }
NMS_API void formatImpl(String& buf, StrView fmt, u8      val) { _formatInt(buf, fmt, val,      "%u");   }
NMS_API void formatImpl(String& buf, StrView fmt, i16     val) { _formatInt(buf, fmt, val,      "%d");   }
NMS_API void formatImpl(String& buf, StrView fmt, u16     val) { _formatInt(buf, fmt, val,      "%u");   }
NMS_API void formatImpl(String& buf, StrView fmt, i32     val) { _formatInt(buf, fmt, val,      "%d");   }
NMS_API void formatImpl(String& buf, StrView fmt, u32     val) { _formatInt(buf, fmt, val,      "%u");   }
NMS_API void formatImpl(String& buf, StrView fmt, i64     val) { _formatInt(buf, fmt, val,      "%lld"); }
NMS_API void formatImpl(String& buf, StrView fmt, u64     val) { _formatInt(buf, fmt, val,      "%llu"); }
NMS_API void formatImpl(String& buf, StrView fmt, void*   val) { _formatInt(buf, fmt, u64(val), "%p");   }
NMS_API void formatImpl(String& buf, StrView fmt, f32     val) { _formatFlt(buf, fmt, val); }
NMS_API void formatImpl(String& buf, StrView fmt, f64     val) { _formatFlt(buf, fmt, val); }

NMS_API void formatImpl(String& buf, StrView fmt, bool    val) {
    if (fmt.count() > 0) {
        if (fmt[0] == 'C') {
            buf += val ? cstr("True") : cstr("False");
            return;
        }
        if (fmt[0] == 'U') {
            buf += val ? cstr("TRUE") : cstr("FALSE");
            return;
        }
    }
    buf += val ? cstr("true") : cstr("false");
}

NMS_API void formatImpl(String& buf, StrView fmt, const IException& val) {
    (void)fmt;
    val.format(buf);
}

NMS_API bool Formatter::next(u32& id, StrView& fmt) {

    fmt = StrView();

    const auto old_len = buff_.count();
    buff_.reserve(old_len + fmts_.count());
    auto new_len = old_len;
    auto new_dat = buff_.data();

    // find {
    for (u32 i = 0; i < fmts_.count(); ++i) {
        auto c = fmts_[i];

        if (c == '{') {
            if (i + 1 < fmts_.count() && fmts_[i + 1] == '{') {
                new_dat[new_len++] = '{';
                ++i;
                continue;
            }
            else {
                fmts_ = fmts_.slice( i, fmts_.count() - 1);
                break;
            }
        }

        if (c == '}') {
            if (i + 1 < fmts_.count() && fmts_[i + 1] == '}') {
                ++i;
            }
        }

        new_dat[new_len++] = c;

        if (i == fmts_.count() - 1) {
            fmts_ = {};
            buff_.resize(new_len);
            return false;
        }
    }

    // find }
    for (u32 i = 0; i < fmts_.count(); ++i) {
        auto c = fmts_[i];

        if (c == '}') {
            if (i + 1 < fmts_.count() && fmts_[i + 1] == '}') {
                new_dat[new_len++] = '}';
                ++i;
                continue;
            }
            else {
                fmt   = fmts_.slice( 0u,       i);
                fmts_ = fmts_.slice( i + 1, fmts_.count() - 1);
                break;
            }
        }
    }

    if (fmt.count() < 2) {
        fmts_ = {};
        buff_.resize(new_len);
        return false;
    }

    fmt = fmt.slice(1u, fmt.count() - 2);
    if (fmt[0] >= '0' && fmt[0] <= '9') {
        id = 0;
        for (u32 i = 0; i < fmt.count(); ++i) {
            if (fmt[i] >= '0' && fmt[i] <= '9') {
                id = id * 10 + (fmt[i] - '0');
            }
            else {
                fmt = fmt.slice( i, fmt.count() - 1);
            }
        }
    }

    if (fmt[0] == ':') {
        fmt = fmt.slice(1u, fmt.count() - 1);
    }

    buff_.resize(new_len);
    return true;
}

}