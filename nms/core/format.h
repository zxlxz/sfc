#pragma once

#include <nms/core/type.h>
#include <nms/core/cpp.h>
#include <nms/core/view.h>
#include <nms/core/string.h>
#include <nms/core/exception.h>
#include <nms/util/stacktrace.h>

namespace nms
{

template<class T>
auto format_switch(String& buf, StrView fmt, const T& t);

struct IFormatable
{
    template<class T>
    static void _format(String& buf, const T& obj) {
        buf += "{\n";

#define call_do_format(n, ...)    _do_format(I32<n>{}, obj, &buf);
        NMSCPP_LOOP(99, call_do_format);
#undef call_do_format
        buf += "}\n";
    }

private:
    // format-do
    template<class T, i32 I>
     static auto _do_format(I32<I> idx, const T& obj, String* buf) ->$when<(I < T::_$property_cnt)> {
        auto t = (obj)[idx];
        sformat(*buf, "    {}: {}\n", t.name, t.value);
        return;
    }

    // format-end
    template<class T, i32 I >
    static auto _do_format(I32<I>, const T&, ...) -> $when<(I >= T::_$property_cnt)> {
        return;
    }
};

#pragma region format impl

NMS_API void formatImpl(String& buf, StrView fmt, i8       val);
NMS_API void formatImpl(String& buf, StrView fmt, u8       val);
NMS_API void formatImpl(String& buf, StrView fmt, i16      val);
NMS_API void formatImpl(String& buf, StrView fmt, u16      val);
NMS_API void formatImpl(String& buf, StrView fmt, i32      val);
NMS_API void formatImpl(String& buf, StrView fmt, u32      val);
NMS_API void formatImpl(String& buf, StrView fmt, i64      val);
NMS_API void formatImpl(String& buf, StrView fmt, u64      val);
NMS_API void formatImpl(String& buf, StrView fmt, f32      val);
NMS_API void formatImpl(String& buf, StrView fmt, f64      val);
NMS_API void formatImpl(String& buf, StrView fmt, void*    val);
NMS_API void formatImpl(String& buf, StrView fmt, StrView  val);

NMS_API void formatImpl(String& buf, StrView fmt, const IException&  val);

inline  void formatImpl(String& buf, StrView fmt, const String& val) {
    formatImpl(buf, fmt, StrView(val));
}

template<u32 N>
void formatImpl(String& buf, StrView fmt, const char(&v)[N]) {
    formatImpl(buf, fmt, cstr(v));
}

inline void formatImpl(String& buf, StrView fmt, const char* str) {
    formatImpl(buf, fmt, cstr(str));
}


template<class T, u32 N>
void formatImpl(String& buf, StrView fmt, const Vec<T, N>& v) {
    buf += "[";
    for (u32 i = 0; i < N; ++i) {
        format_switch(buf, fmt, v[i]);
        if (i != N - 1) buf += ", ";
    }
    buf += "]";
}

template<class T, u32 N>
void formatImpl(String& buf, StrView fmt, const T(&v)[N]) {
    buf += "[";
    for (u32 i = 0; i < N; ++i) {
        format_switch(buf, fmt, v[i]);
        if (i != N - 1) buf += ", ";
    }
    buf += "]";
}

template<class T, u32 N>
void formatImpl(String& buf, StrView fmt, const List<T, N>& v) {
    const auto n = v.count();
    buf += "[";
    for (u32 i = 0; i < n; ++i) {
        format_switch(buf, fmt, v[i]);
        if (i != n - 1) buf += ", ";
    }
    buf += "]";
}


/*!
* format:
* type: view<T,1>
* fmt:  ?...?
*/
template<class T>
void formatImpl(String& buf, StrView fmt, const View<T, 1>& v) {
    buf.reserve(buf.count() + v.count() * 8);

    const auto nx = v.count();
    const auto delimiters = fmt.count() == 0 ? StrView{ ", " } : fmt;

    for (u32 x = 0; x < nx; ++x) {
        format_switch(buf, fmt, v(x));
        if (x != nx - 1) {
            buf += delimiters;
        }
    }
}


/* format: view<T,2> */
template<class T>
void formatImpl(String& buf, StrView fmt, const View<T, 2>& v) {
    buf.reserve(buf.count() + v.count() * 8);

    const auto vmod = fmt == "|";
    const auto fmt0 = vmod ? fmt.slice(1, -1) : fmt;

    if (vmod) {
        buf += StrView{ "\n" };
    }
    for (u32 i1 = 0; i1 < v.size(1); ++i1) {
        buf += vmod ? StrView{ "    [" } : StrView{ "[" };
        format_switch(buf, fmt0, v.slice({ 0, -1 }, { i1 }));
        buf += "]";
        if (i1 < v.size(1) - 1) {
            buf += vmod ? StrView{ ",\n" } : StrView{ "," };
        }
    }
}

#pragma endregion

#pragma region format switch

template<class T>
__forceinline auto _format_switch(String& buf, StrView fmt, const T& t, Version<5>) -> decltype(t.format(buf, fmt)) {
    return t.format(buf, fmt);
}

template<class T>
__forceinline auto _format_switch(String& buf, StrView fmt, const T& t, Version<4>) -> decltype(t.format(buf)) {
    (void)fmt;
    return t.format(buf);
}

template<class T>
__forceinline auto _format_switch(String& buf, StrView fmt, const T& t, Version<3>) -> decltype(formatImpl(buf, fmt, t)) {
    return formatImpl(buf, fmt, t);
}

template<class T>
__forceinline auto _format_switch(String& buf, StrView fmt, const T& t, Version<2>) -> $when<$is_base_of<IFormatable, T>> {
    (void)fmt;
    return IFormatable::_format(buf, t);
}

template<class T>
__forceinline auto _format_switch(String& buf, StrView fmt, const T& t, Version<1>) -> decltype(static_cast<StrView>(t), 0) {
    auto str = static_cast<StrView>(t);
    formatImpl(buf, fmt, str);
    return 0;
}

template<class T>
__forceinline auto _format_switch(String& buf, StrView fmt, const T& t, Version<0>) -> $when<$is_enum<T>> {
    auto str = mkEnum(t).name();
    formatImpl(buf, fmt, str);
    return;
}


template<class T>
__forceinline auto format_switch(String& buf, StrView fmt, const T& t) {
    return _format_switch(buf, fmt, t, Version<5>{});
}

template<class T>
String tostr(const T& t) {
    String buf;
    format_switch(buf, StrView{}, t);
    return buf;
}

#pragma endregion

class Formatter
{
public:
    class EOutOfRange : public IException
    {};

    Formatter(String& buff, StrView fmts)
        : buff_(buff), fmts_(fmts)
    {}

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

    NMS_API bool next(u32& id, StrView& fmt);

    void doFormat(i32 id, StrView fmt) const {
        NMS_THROW(EOutOfRange{});
    }

    template<class T, class ...U>
    void doFormat(i32 id, StrView fmt, const T& t, const U& ...u) {
        if (id == 0) {
            format_switch(buff_, fmt, t);
        }
        else {
            doFormat(id - 1, fmt, u...);
        }
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

}