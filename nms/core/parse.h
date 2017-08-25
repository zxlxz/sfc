#pragma once

#include <nms/core/type.h>
#include <nms/core/view.h>
#include <nms/core/string.h>
#include <nms/core/exception.h>
#include <nms/util/stacktrace.h>

namespace nms
{

class EParseFailed
    : public IException
{};


#pragma region parse switch
template<class T>
__forceinline auto _parse_switch(StrView buf, StrView fmt, T& t, Version<5>) -> decltype(t.parse(buf, fmt), 0) {
    t = t.parse(buf, fmt);
    return 0;
}

template<class T>
__forceinline auto _parse_switch(StrView buf, StrView fmt, T& t, Version<4>) -> decltype(t.parse(buf), 0) {
    t = t.parse(buf);
    return 0;
}

template<class T>
__forceinline auto _parse_switch(StrView buf, StrView fmt, T& t, Version<3>) -> decltype(parseImpl(buf, fmt, t), 0) {
    parseImpl(buf, fmt, t);
    return 0;
}

template<class T>
__forceinline auto _parse_switch(StrView buf, StrView fmt, T& t, Version<2>) -> $when<$is_enum<T>> {
    t = Enum<T>::parse(buf);
    return 0;
}

template<class T>
__forceinline auto parse_switch(StrView buf, StrView fmt, T& t) {
    return _parse_switch(buf, fmt, t, Version<5>{});
}

template<class T>
auto parse(StrView str, StrView fmt = {}) {
    T    val;
    auto ret = parse_switch(str, fmt, val);

    if (ret!=0) {
        NMS_THROW(EParseFailed{});
    }
    return val;
}

#pragma endregion

#pragma region parse impl
NMS_API int parseImpl(StrView str, StrView fmt, u8&   val);
NMS_API int parseImpl(StrView str, StrView fmt, i8&   val);
NMS_API int parseImpl(StrView str, StrView fmt, u16&  val);
NMS_API int parseImpl(StrView str, StrView fmt, i16&  val);
NMS_API int parseImpl(StrView str, StrView fmt, u32&  val);
NMS_API int parseImpl(StrView str, StrView fmt, i32&  val);
NMS_API int parseImpl(StrView str, StrView fmt, u64&  val);
NMS_API int parseImpl(StrView str, StrView fmt, i64&  val);
NMS_API int parseImpl(StrView str, StrView fmt, f32&  val);
NMS_API int parseImpl(StrView str, StrView fmt, f64&  val);
#pragma endregion

}
