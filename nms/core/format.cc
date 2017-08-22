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
static void _formatStr(String& buf, StrView val, StrView sfmt) {
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
static void _formatInt(String& buf, T val, StrView sfmt, StrView type) {
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
static void _formatFlt(String& buf, T val, FmtSpec fmt) {
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

NMS_API void formatImpl(String& buf, StrView val, StrView fmt) { _formatStr(buf, val, fmt);              }
NMS_API void formatImpl(String& buf, cstr_t  val, StrView fmt) { _formatStr(buf, cstr(val), fmt);        }

NMS_API void formatImpl(String& buf, i8      val, StrView fmt) { _formatInt(buf, val,      fmt, "%d");   }
NMS_API void formatImpl(String& buf, u8      val, StrView fmt) { _formatInt(buf, val,      fmt, "%u");   }
NMS_API void formatImpl(String& buf, i16     val, StrView fmt) { _formatInt(buf, val,      fmt, "%d");   }
NMS_API void formatImpl(String& buf, u16     val, StrView fmt) { _formatInt(buf, val,      fmt, "%u");   }
NMS_API void formatImpl(String& buf, i32     val, StrView fmt) { _formatInt(buf, val,      fmt, "%d");   }
NMS_API void formatImpl(String& buf, u32     val, StrView fmt) { _formatInt(buf, val,      fmt, "%u");   }
NMS_API void formatImpl(String& buf, i64     val, StrView fmt) { _formatInt(buf, val,      fmt, "%lld"); }
NMS_API void formatImpl(String& buf, u64     val, StrView fmt) { _formatInt(buf, val,      fmt, "%llu"); }
NMS_API void formatImpl(String& buf, void*   val, StrView fmt) { _formatInt(buf, u64(val), fmt, "%p");   }

NMS_API void formatImpl(String& buf, f32     val, StrView fmt) { _formatFlt(buf, val,      fmt.count()==0 ? FmtSpec(6, 3) : FmtSpec(fmt)); }
NMS_API void formatImpl(String& buf, f64     val, StrView fmt) { _formatFlt(buf, val,      fmt.count()==0 ? FmtSpec(9, 6) : FmtSpec(fmt)); }

NMS_API void formatImpl(String& buf, const IException& val, StrView fmt) {
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


template<class T>
static bool _parse(StrView str, T& val, const char* fmt) {
    const auto buf = str.data();

#ifdef _MSC_VER
    const auto cnt = str.count();
    const auto ret = _snscanf_s(buf, cnt, fmt, &val);
#else
    const auto ret = sscanf(buf, fmt, &val);
#endif
    return ret != -1;
}

NMS_API bool parse(StrView str, u8&  val, StrView fmt) { return _parse(str, val, "%hhu"); }
NMS_API bool parse(StrView str, i8&  val, StrView fmt) { return _parse(str, val, "%hhd"); }
NMS_API bool parse(StrView str, u16& val, StrView fmt) { return _parse(str, val, "%hu"); }
NMS_API bool parse(StrView str, i16& val, StrView fmt) { return _parse(str, val, "%hd"); }
NMS_API bool parse(StrView str, u32& val, StrView fmt) { return _parse(str, val, "%u"); }
NMS_API bool parse(StrView str, i32& val, StrView fmt) { return _parse(str, val, "%d"); }
NMS_API bool parse(StrView str, u64& val, StrView fmt) { return _parse(str, val, "%Lu"); }
NMS_API bool parse(StrView str, i64& val, StrView fmt) { return _parse(str, val, "%Ld"); }
NMS_API bool parse(StrView str, f32& val, StrView fmt) { return _parse(str, val, "%f"); }
NMS_API bool parse(StrView str, f64& val, StrView fmt) { return _parse(str, val, "%lf"); }

}