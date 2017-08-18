#pragma once

#include <nms/core/type.h>
#include <nms/core/cpp.h>
#include <nms/core/view.h>
#include <nms/core/string.h>
#include <nms/core/exception.h>

namespace nms
{

using cstr_t = const char*;

struct IFormatable
{
    friend class Formatter;

protected:
    template<class T>
    static void _format(String& buf, const T& obj) {
        buf += "{\n";

#define call_do_format(n, ...)    _do_format(I32<n>{}, obj, &buf);
        NMSCPP_LOOP(99, call_do_format)
#undef call_do_format

        buf += "}\n";
    }

private:
    // format-do
    template<class T, i32 I, class = $when<(I<T::_$property_cnt)> >
    static void _do_format(I32<I> idx, const T& obj, String* buf) {
        auto t = (obj)[idx];
        sformat(*buf, "    {}: {}\n", t.name, t.value);
    }

    // format-end
    template<class T, i32 I, class = $when<(I>=T::_$property_cnt)> >
    static void _do_format(I32<I>, const T&, ...)
    {}
};

/* format value */
NMS_API void formatImpl(String& buf, i8       val, StrView fmt);
NMS_API void formatImpl(String& buf, u8       val, StrView fmt);
NMS_API void formatImpl(String& buf, i16      val, StrView fmt);
NMS_API void formatImpl(String& buf, u16      val, StrView fmt);
NMS_API void formatImpl(String& buf, i32      val, StrView fmt);
NMS_API void formatImpl(String& buf, u32      val, StrView fmt);
NMS_API void formatImpl(String& buf, i64      val, StrView fmt);
NMS_API void formatImpl(String& buf, u64      val, StrView fmt);
NMS_API void formatImpl(String& buf, f32      val, StrView fmt);
NMS_API void formatImpl(String& buf, f64      val, StrView fmt);
NMS_API void formatImpl(String& buf, void*    val, StrView fmt);
NMS_API void formatImpl(String& buf, StrView  val, StrView fmt);
NMS_API void formatImpl(String& buf, cstr_t   val, StrView fmt);

NMS_API void formatImpl(String& buf, const IException&  val, StrView fmt);

inline void formatImpl(String& buf, const String& val, StrView fmt) {
    formatImpl(buf, val.operator StrView(), fmt);
}

template<u32 N>
void formatImpl(String& buf, const char(&v)[N], StrView fmt) {
    formatImpl(buf, cstr(v), fmt);
}


template<class T, u32 N>
void formatImpl(String& buf, const Vec<T, N>& v, StrView fmt) {
    buf += "[";
    for (u32 i = 0; i < N; ++i) {
        formatImpl(buf, v[i], fmt);
        if (i != N - 1) buf += ", ";
    }
    buf += "]";
}

template<class T, u32 N>
void formatImpl(String& buf, const T(&v)[N], StrView fmt) {
    buf += "[";
    for(u32 i = 0; i < N; ++i) {
        formatImpl(buf, v[i], fmt);
        if (i!=N-1) buf += ", ";
    }
    buf += "]";
}

template<class T, u32 N>
void formatImpl(String& buf, const List<T, N>& v, StrView fmt) {
    const auto n = v.count();
    buf += "[";
    for (u32 i = 0; i < n; ++i) {
        formatImpl(buf, v[i], fmt);
        if (i != n - 1) buf += ", ";
    }
    buf += "]";
}

class Formatter
{
public:
    class EOutOfRange : public IException
    {};

    Formatter(String& buff, StrView fmts)
        : buff_(buff), fmts_(fmts)
    {}

    NMS_API bool next(u32& id, StrView& fmt);

    template<class ...U>
    void operator()(const U& ...u) {
        u32     id = 0;
        StrView fmt;

        while (next(id, fmt)) {
            doFormat(id, fmt, u...);
            ++id;
        }
    }

protected:
    String& buff_;
    StrView fmts_;

    void doFormat(i32 id, StrView fmt) const {
        throw EOutOfRange{};
    }

    template<class T, class ...U>
    void doFormat(i32 id, StrView fmt, const T& t, const U& ...u) {
        if (id == 0) {
            format_select(t, fmt, Version<1>{});
        }
        else {
            doFormat(id - 1, fmt, u...);
        }
    }

    template<class T>
    __forceinline auto format_select(const T& t, StrView fmt, Version<1>) -> decltype(t.format(buff_, fmt)) {
        return t.format(buff_, fmt);
    }

    template<class T>
    __forceinline auto format_select(const T& t, StrView fmt, Version<1>) -> decltype(t.format(buff_)) {
        return t.format(buff_);
    }

    template<class T>
    __forceinline auto format_select(const T& t, StrView fmt, Version<0>) -> $when<$is_base_of<IFormatable, T>> {
        (void)fmt;
        return IFormatable::_format(buff_, t);
    }

    template<class T>
    __forceinline auto format_select(const T& t, StrView fmt, ...) ->decltype(formatImpl(buff_, t, fmt)) {
        return formatImpl(buff_, t, fmt);
    }
};

template<class ...T>
void sformat(String& buf, StrView fmt, const T& ...t) {
    Formatter fmtter(buf, fmt);
    fmtter(t...);
}

/* format to string */
template<class ...T>
String format(StrView fmt, const T& ...t) {
    String buf = {};
    Formatter fmtter(buf, fmt);
    fmtter(t...);
    return buf;
}

NMS_API bool parse(StrView str, u8&   val, StrView fmt={});
NMS_API bool parse(StrView str, i8&   val, StrView fmt={});
NMS_API bool parse(StrView str, u16&  val, StrView fmt={});
NMS_API bool parse(StrView str, i16&  val, StrView fmt={});
NMS_API bool parse(StrView str, u32&  val, StrView fmt={});
NMS_API bool parse(StrView str, i32&  val, StrView fmt={});
NMS_API bool parse(StrView str, u64&  val, StrView fmt={});
NMS_API bool parse(StrView str, i64&  val, StrView fmt={});
NMS_API bool parse(StrView str, f32&  val, StrView fmt={});
NMS_API bool parse(StrView str, f64&  val, StrView fmt={});

class EParseFailed
    : public IException
{};


template<class T>
T parse(StrView str, StrView fmt = {}) {
    T    val;
    auto ret = parse(str, val, fmt);

    if (!ret) {
        throw EParseFailed{};
    }
    return val;
}

/*!
 * format: 
 * type: view<T,1> 
 * fmt:  ?...?
 */
template<class T>
void formatImpl(String& buf, const View<T, 1>& v, StrView fmt) {
    buf.reserve(buf.count() + v.count() * 8);

    const auto nx         = v.count();
    const auto delimiters = fmt.count() == 0 ? StrView{ ", " } : fmt;

    for (u32 x = 0; x < nx; ++x) {
        formatImpl(buf, v(x), fmt);
        if (x != nx - 1) {
            buf += delimiters;
        }
    }
}


/* format: view<T,2> */
template<class T>
void formatImpl(String& buf, const View<T, 2>& v, StrView fmt) {
    buf.reserve(buf.count() + v.count() * 8);

    const auto vmod = fmt == "|";
    const auto fmt0 = vmod ? fmt.slice(1, -1) : fmt;

    if (vmod) {
        buf += StrView{ "\n" };
    }
    for (u32 i1 = 0; i1 < v.size(1); ++i1) {
        buf += vmod ? StrView{ "    [" } : StrView{ "[" };
        formatImpl(buf, v.slice({ 0, -1 }, { i1 }), fmt0);
        buf += "]";
        if (i1 < v.size(1) - 1) {
            buf += vmod ? StrView{ ",\n" } : StrView{ "," };
        }
    }
}

}