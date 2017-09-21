#pragma once

#include <nms/core/cpp.h>
#include <nms/core/trait.h>
#include <nms/core/string.h>
#include <nms/core/exception.h>


namespace nms
{

struct IFormatable
{
    template<class T>
    static void _format(IString& buf, const T& obj) {
        IFormatable self;

        auto name_len = 4u;
        self._do_format(nullptr, obj, name_len);
        name_len = (name_len + 3) / 4 * 4;  // 4 spaces indent
        self._do_format(&buf, obj, name_len);
    }

private:
    template<class T>
    void _do_format(IString* buf, const T& obj, u32& name_len) const {
        if (buf != nullptr) *buf += "{\n";

#define call_do_format(n, ...)    this->_do_format_switch(Ti32<n>{}, buf, obj, name_len);
        { NMSCPP_LOOP(99, call_do_format, ~) }
#undef call_do_format
        if (buf != nullptr) *buf += "}\n";
    }

    // format-do
    template<class T, i32 I, class = $when<(I + 1 <= T::_$property_cnt)> >
    void _do_format_switch(Ti32<I> idx, IString* buf, const T& obj, u32& name_len) const {
        _do_format_property(idx, buf, obj, name_len);
    }

    // format-end
    template<class T, i32 I, class = $when<(I + 0 >= T::_$property_cnt)> >
    void _do_format_switch(Ti32<I>, IString*, const T&, ...) const {
    }

    template<class T, i32 I>
    void _do_format_property(Ti32<I> idx, IString* buf, const T& obj, u32& name_len) const;
};

#pragma region format: basic types
NMS_API void _format(IString& buf, const StrView& fmt, i8       val);
NMS_API void _format(IString& buf, const StrView& fmt, u8       val);
NMS_API void _format(IString& buf, const StrView& fmt, i16      val);
NMS_API void _format(IString& buf, const StrView& fmt, u16      val);
NMS_API void _format(IString& buf, const StrView& fmt, i32      val);
NMS_API void _format(IString& buf, const StrView& fmt, u32      val);
NMS_API void _format(IString& buf, const StrView& fmt, i64      val);
NMS_API void _format(IString& buf, const StrView& fmt, u64      val);
NMS_API void _format(IString& buf, const StrView& fmt, f32      val);
NMS_API void _format(IString& buf, const StrView& fmt, f64      val);
NMS_API void _format(IString& buf, const StrView& fmt, StrView  val);
NMS_API void _format(IString& buf, const StrView& fmt, bool     val);

#ifdef NMS_STDCXX_TYPEINFO
NMS_API void _format(IString& buf, const StrView& fmt, decltype(typeid(void)) val);
#endif

NMS_API void _format(IString& buf, const StrView& fmt, const IException&  val);

inline  void _format(IString& buf, const StrView& fmt, const void* val) {
    fmt.count() == 0
        ? _format(buf, "0x{:x}", reinterpret_cast<u64>(val))
        : _format(buf, fmt, reinterpret_cast<u64>(val));
}

inline  void _format(IString& buf, const StrView& fmt, const IString& val) {
    _format(buf, fmt, StrView(val));
}

template<u32 N>
inline  void _format(IString& buf, const StrView& fmt, const char(&v)[N]) {
    _format(buf, fmt, StrView{ v });
}

inline  void _format(IString& buf, const StrView& fmt, const char* str) {
    _format(buf, fmt, StrView{ str, u32(strlen(str)) });
}

#pragma endregion

#pragma region format switch
#ifndef NMS_CC_INTELLISENSE

template<class T>
__forceinline auto _format_switch(IString& buf, const StrView& fmt, const T& t, Tver<5>) -> decltype(t.format(buf, fmt)) {
    return t.format(buf, fmt);
}

template<class T>
__forceinline auto _format_switch(IString& buf, const StrView& fmt, const T& t, Tver<4>) -> decltype(t.format(buf)) {
    (void)fmt;
    return t.format(buf);
}

template<class T>
__forceinline auto _format_switch(IString& buf, const StrView& fmt, const T& t, Tver<3>) -> decltype(_format(buf, fmt, t)) {
    return _format(buf, fmt, t);
}

template<class T, class = $when_is<$enum, T> >
__forceinline auto _format_switch(IString& buf, const StrView& fmt, const T& t, Tver<2>) {
    auto str = mkEnum(t).name();
    if (str.count() != 0) {
        _format(buf, fmt, str);
    }
    else {
        buf += typeof<T>().name();
        buf += ".";
        _format(buf, fmt, u32(t));
    }
    return;
}

template<class T, class = $when_is<IFormatable, T> >
__forceinline auto _format_switch(IString& buf, const StrView& /*fmt*/, const T& t, Tver<1>) {
    return IFormatable::_format(buf, t);
}

template<class T, class = $when_as<StrView, T> >
__forceinline auto _format_switch(IString& buf, const StrView& fmt, const T& t, Tver<0>) {
    auto str = static_cast<StrView>(t);
    _format(buf, fmt, str);
    return 0;
}

#endif

template<class T>
void format_switch(IString& buf, const StrView& fmt, const T& t) {
#ifndef NMS_CC_INTELLISENSE
    _format_switch(buf, fmt, t, Tver<5>{});
#else
    (void)buf;
    (void)fmt;
    (void)t;
#endif
}
#pragma endregion

#pragma region format: array
template<class T, u32 N>
void _format(IString& buf, const StrView& fmt, const Vec<T, N>& v) {
    buf += "[";
    for (u32 i = 0; i < N; ++i) {
        format_switch(buf, fmt, v[i]);
        if (i != N - 1) buf += ", ";
    }
    buf += "]";
}

template<class T, u32 N>
void _format(IString& buf, const StrView& fmt, const T(&v)[N]) {
    buf += "[";
    for (u32 i = 0; i < N; ++i) {
        format_switch(buf, fmt, v[i]);
        if (i != N - 1) buf += ", ";
    }
    buf += "]";
}

template<class T, u32 N>
void _format(IString& buf, const StrView& fmt, const View<T, N>& v) {
    const auto n = v.count();
    buf += "[";
    for (u32 i = 0; i < n; ++i) {
        format_switch(buf, fmt, v[i]);
        if (i != n - 1) buf += ", ";
    }
    buf += "]";
}

/*! format: view<T,1> */
template<class T>
void _format(IString& buf, const StrView& fmt, const View<T, 1>& v1) {
    static const StrView delimiter = ", ";
    buf.reserve(buf.count() + v1.count() * 8);

    const auto nx = v1.count();

    for (u32 x = 0; x < nx; ++x) {
        format_switch(buf, fmt, v1(x));
        if (x != nx - 1) {
            buf += delimiter;
        }
    }
}

/* format: view<T,2> */
template<class T>
void _format(IString& buf, const StrView& fmt, const View<T, 2>& v2) {
    buf.reserve(buf.count() + v2.count() * 8);

    const auto cnt = v2.size(1);

    buf += StrView{ "\n" };
    for (u32 i1 = 0; i1 < cnt; ++i1) {
        buf += StrView{ "    |" };
        auto v1 = v2.slice({ 0, -1 }, { i1 });
        _format(buf, fmt, v1);
        if (i1 + 1 != cnt) {
            buf += "|\n";
        }
        else {
            buf += "|";
        }
    }
}

#pragma endregion

#pragma region format: object
template<class T, i32 I>
void IFormatable::_do_format_property(Ti32<I> idx, IString* buf, const T& obj, u32& name_len) const {
    auto t = (obj)[idx];
    auto& name  = t.name;
    auto& value = (*t.pval);
    if (buf != nullptr) {
        buf->appends(4, ' ');
        *buf += name;
        buf->appends(name_len - name.count(), ' ');
        *buf += StrView(": ");
        format_switch(*buf, {}, value);
        *buf += StrView("\n");
    }
    else {
        if (name_len < name.count()) {
            name_len = name.count();
        }
    }
    return;
}
#pragma endregion

#pragma region formatter
class Formatter
{
public:
    Formatter(IString* pbuf, const StrView& fmts)
        : pbuf_(pbuf), fmts_(fmts)
    {}

    template<class ...U>
    void operator()(const U& ...u) {
        u32     id  = 0u;
        StrView fmt = fmts_;

        while (next(&id, &fmt)) {
            _doFormat(id, fmt, u...);
            ++id;
        }
    }

protected:
    IString* pbuf_;
    StrView fmts_;

    NMS_API bool next(u32* id, StrView* fmt);

    void _doFormat(u32 /*id*/, const StrView& /*fmt*/) const {
    }

    template<class T, class ...U>
    void _doFormat(u32 id, const StrView& fmt, const T& t, const U& ...u) {
        if (id == 0) {
            format_switch(*pbuf_, fmt, t);
        }
        else {
            _doFormat(id - 1, fmt, u...);
        }
    }
};

#pragma endregion

template<class ...T>
void sformat(IString& buf, const StrView& fmt, const T& ...t) {
    Formatter fmtter(&buf, fmt);
    fmtter(t...);
}

/* format to string */
template<class ...T>
U8String<256> format(const StrView& fmt, const T& ...t) {
    U8String<256> buf = {};
    sformat(buf, fmt, t...);
    return buf;
}

}
