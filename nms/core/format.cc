#include <nms/core.h>

namespace nms
{

/* === format spec === */
struct FmtSpec
{
    i8      align   = '\0';
    i8      sign    = '\0';
    i8      type    = '\0';
    i32     prec    = -1;
    u32     width   = 0;

    explicit FmtSpec(const StrView& fmt) {
        if (fmt.count() == 0) {
            return;
        }

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
            width = width * 10 + u32(fmt[i++] - '0');
        }

        // parse: spec
        if (i < n && (fmt[i] == '.')) {
            ++i;

            prec = 0;
            while (i < n && ('0' <= fmt[i] && fmt[i] <= '9')) {
                prec = prec * 10 + i32(fmt[i++] - '0');
            }
        }

        // parse: type
        if (i < n) {
            type = fmt[i];
        }
    }
};

// [<>=][width]
static void _formatStr(IString& buf, const StrView& sfmt, const StrView& val) {
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

static inline auto snprint_int(char buff[], u64 buff_size, u32 val) {
    return ::snprintf(buff, buff_size, "%u", val);
}

static inline auto snprint_int(char buff[], u64 buff_size, i32 val) {
    return ::snprintf(buff, buff_size, "%u", val);
}

static inline auto snprint_int(char buff[], u64 buff_size, u64 val) {
    return ::snprintf(buff, buff_size, "%llu", ullong(val));
}

static inline auto snprint_int(char buff[], u64 buff_size, i64 val) {
    return ::snprintf(buff, buff_size, "%lld", llong(val));
}

// [align:<>=][width:number].[prec:number]s
template<class T>
static void _formatInt(IString& str_out, const StrView& sfmt, const T& val_ori) {
    char str_body[256];
    auto str_len = 0u;

    if (sfmt.count()==0) {
        const auto num_cnt = snprint_int(str_body, sizeof(str_body), val_ori);
        if (num_cnt > 0) {
            str_out.appends(str_body, u32(num_cnt));
        }
    }
    else {
        const auto  val_abs = val_ori < 0 ? 0 - val_ori : val_ori;

        const FmtSpec fmt(sfmt);

        switch (fmt.type) {
        case 'c':   // character
            str_body[str_len++] = char(val_ori);
            break;

        case ' ':   // blanks
            str_len = u32(val_ori) < 256u ? u32(val_ori) : 256u;
            ::memset(str_body, ' ', str_len);
            break;

        default:
            if (val_ori < 0) {
                str_body[str_len++] = '-';
            }
            else {
                if (fmt.sign == '+') { str_body[str_len++] = '+'; }
                if (fmt.sign == '-') { str_body[str_len++] = ' '; }
            }
            auto num_cnt = snprint_int(str_body + str_len, sizeof(str_body) - str_len, val_abs);
            if (num_cnt > 0) {
                str_len += u32(num_cnt);
            }
            break;
        }

        if (fmt.width <= str_len) {
            str_out.appends(str_body, str_len);
        }
        else {
            str_out.reserve(str_out.count());

            switch (fmt.align) {
            case '<':
                str_out.appends(str_body, str_len);
                str_out.appends(fmt.width - str_len, ' ');
                break;
            case '>': default:
                str_out.appends(fmt.width - str_len, ' ');
                str_out.appends(str_body, str_len);
                break;
            case '^':
                str_out.appends((fmt.width - str_len + 0) / 2, ' ');
                str_out.appends(str_body, str_len) ;
                str_out.appends((fmt.width - str_len + 1) / 2, ' ');
                break;
            }
        }
    }
}

// [align:<>=][sign:+-][width:number].[prec:number]
template<class T>
static void _formatFlt(IString& str_out, const StrView& str_fmt, const T& val_ori) {
    FmtSpec fmt(str_fmt);

    // set default prec
    // float:  3
    // double: 6
    if (fmt.prec == -1) {
        fmt.prec = $is<float, T> ? 3 : 6;
        if (fmt.type == 'g') {
            fmt.prec += 3;
        }
    }

    const auto val_abs = val_ori < 0 ? -val_ori : +val_ori;

    char str_body[256];
    auto str_head = str_body + 1;
    auto str_cnt  = fmt.type == 'g'
        ? snprintf(str_head, sizeof(str_body) - 1, "%.*g", fmt.prec, val_abs)
        : snprintf(str_head, sizeof(str_body) - 1, "%.*f", fmt.prec, val_abs);

    auto str_len = str_cnt > 0 ? u32(str_cnt) : 0;

    if (val_ori < 0) {
        str_body[0] = '-';
        ++str_len;
        --str_head;
    }
    else {
        if (fmt.sign == '+') { str_body[0] = '+'; ++str_len; --str_head; }
        if (fmt.sign == '-') { str_body[0] = ' '; ++str_len; --str_head; }
    }

    if (str_len >= fmt.width) {
        str_out.appends(str_head, str_len);
    }
    else {
        str_out.reserve(str_out.count() + fmt.width);

        switch (fmt.align) {
        case '<':
            str_out.appends(str_head, str_len);
            str_out.appends(fmt.width - str_len, ' ');
            break;
        case '>': default:
            str_out.appends(fmt.width - str_len, ' ');
            str_out.appends(str_head, str_len);
            break;
        case '^':
            str_out.appends((fmt.width - str_len + 0) / 2, ' ');
            str_out.appends(str_head, str_len);
            str_out.appends((fmt.width - str_len + 1) / 2, ' ');
            break;
        }
    }
}

NMS_API void _format(IString& buf, const StrView& fmt, StrView val) { _formatStr(buf, fmt, val); }

NMS_API void _format(IString& buf, const StrView& fmt, i8      val) { _formatInt(buf, fmt, val); }
NMS_API void _format(IString& buf, const StrView& fmt, u8      val) { _formatInt(buf, fmt, val); }
NMS_API void _format(IString& buf, const StrView& fmt, i16     val) { _formatInt(buf, fmt, val); }
NMS_API void _format(IString& buf, const StrView& fmt, u16     val) { _formatInt(buf, fmt, val); }
NMS_API void _format(IString& buf, const StrView& fmt, i32     val) { _formatInt(buf, fmt, val); }
NMS_API void _format(IString& buf, const StrView& fmt, u32     val) { _formatInt(buf, fmt, val); }
NMS_API void _format(IString& buf, const StrView& fmt, i64     val) { _formatInt(buf, fmt, val); }
NMS_API void _format(IString& buf, const StrView& fmt, u64     val) { _formatInt(buf, fmt, val); }
NMS_API void _format(IString& buf, const StrView& fmt, f32     val) { _formatFlt(buf, fmt, val); }
NMS_API void _format(IString& buf, const StrView& fmt, f64     val) { _formatFlt(buf, fmt, val); }

NMS_API void _format(IString& buf, const StrView& fmt, bool val) {
    if (fmt.count() > 0) {
        if (fmt[0] == 'C') {
            buf += val ? StrView("True") : StrView("False");
            return;
        }
        if (fmt[0] == 'U') {
            buf += val ? StrView("TRUE") : StrView("FALSE");
            return;
        }
    }
    buf += val ? StrView("true") : StrView("false");
}

NMS_API void _format(IString& buf, const StrView& fmt, decltype(typeid(void)) val) {
    auto raw_name = val.name();
#ifdef NMS_CC_MSVC
    _format(buf, fmt, mkStrView(raw_name));
#else
    char    out_name[4096];
    size_t  length = 4096;
    int     status = 0;
    auto cxx_buff = abi::__cxa_demangle(raw_name, out_name, &length, &status);
    _format(buf, fmt, mkStrView(cxx_buff));
    if (cxx_buff != out_name) {
        ::free(cxx_buff);
    }
#endif
}

NMS_API void _format(IString& buf, const StrView& fmt, const IException& val) {
    (void)fmt;
    val.format(buf);
}

NMS_API bool Formatter::next(u32* pid, StrView* pfmt) {
    auto& id    = *pid;
    auto& fmt   = *pfmt;

    fmt = StrView();

    const auto old_len = pbuf_->count();
    pbuf_->reserve(old_len + fmts_.count());
    auto new_len = old_len;
    auto new_dat = pbuf_->data();

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
            pbuf_->_resize(new_len);
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
        pbuf_->_resize(new_len);
        return false;
    }

    fmt = fmt.slice(1u, fmt.count() - 2);
    if (fmt[0] >= '0' && fmt[0] <= '9') {
        id = 0;
        for (u32 i = 0; i < fmt.count(); ++i) {
            if (fmt[i] >= '0' && fmt[i] <= '9') {
                id = id * 10 + u32(fmt[i] - '0');
            }
            else {
                fmt = fmt.slice( i, fmt.count() - 1);
            }
        }
    }

    if (fmt[0] == ':') {
        fmt = fmt.slice(1u, fmt.count() - 1);
    }

    pbuf_->_resize(new_len);
    return true;
}

}