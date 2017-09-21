#pragma once

#include <nms/core/string.h>

namespace nms
{

class EParseFailed
    : public IException
{};

#pragma region parse impl
NMS_API int _parse(StrView str, StrView fmt, u8&   val);
NMS_API int _parse(StrView str, StrView fmt, i8&   val);
NMS_API int _parse(StrView str, StrView fmt, u16&  val);
NMS_API int _parse(StrView str, StrView fmt, i16&  val);
NMS_API int _parse(StrView str, StrView fmt, u32&  val);
NMS_API int _parse(StrView str, StrView fmt, i32&  val);
NMS_API int _parse(StrView str, StrView fmt, u64&  val);
NMS_API int _parse(StrView str, StrView fmt, i64&  val);
NMS_API int _parse(StrView str, StrView fmt, f32&  val);
NMS_API int _parse(StrView str, StrView fmt, f64&  val);
#pragma endregion

#pragma region parse switch
#ifndef NMS_CC_INTELLISENSE
template<class T>
__forceinline auto _parse_switch(StrView buf, StrView fmt, T& t, Tver<5>) -> decltype(t.parse(buf, fmt), 0) {
    t = t.parse(buf, fmt);
    return 0;
}

template<class T>
__forceinline auto _parse_switch(StrView buf, StrView /*fmt*/, T& t, Tver<4>) -> decltype(t.parse(buf), 0) {
    t = t.parse(buf);
    return 0;
}

template<class T>
__forceinline auto _parse_switch(StrView buf, StrView fmt, T& t, Tver<3>) -> decltype(_parse(buf, fmt, t), 0) {
    _parse(buf, fmt, t);
    return 0;
}

template<class T>
__forceinline auto _parse_switch(StrView buf, StrView /*fmt*/, T& t, Tver<2>) -> $when_is<$enum, T> {
    t = Enum<T>::parse(buf);
    return 0;
}
#endif

template<class T>
__forceinline void parse_switch(StrView buf, StrView fmt, T& t) {
#ifndef NMS_CC_INTELLISENSE
    _parse_switch(buf, fmt, t, Tver<5>{});
#endif
}

#pragma endregion

template<class T>
T& parse(const StrView& str, T& value) {
    if (str.count() != 0) {
        parse_switch(str, {}, value);
    }
    return value;
}

template<class T>
T parse(const StrView& str, const T& default_val = T{}) {
    T value(default_val);
    parse_switch(str, {}, value);
    return value;
}

}
