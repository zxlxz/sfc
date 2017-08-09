#pragma once

#include <nms/core/type.h>
#include <nms/core/trait.h>
#include <nms/core/cpp.h>

namespace nms
{

template<class T>
struct Property
{
    StrView name;
    T       value;

    __forceinline constexpr Property(StrView name, T value)
        : name(name), value(value)
    {}
};

template<class T>
__forceinline Property<T&> make_property(StrView name, T& member) {
    return { name, member };
}

template<class T>
__forceinline Property<const T&> make_property(StrView name, const T& member) {
    return { name, member };
}

static constexpr auto $property_idx = 0;

}

#define NMS_PROPERTY_BEGIN enum:i32 { $property_idx = __COUNTER__ + 1 }
#define NMS_PROPERTY_END   enum:i32 { $property_cnt = __COUNTER__ - $property_idx}

#define NMS_PROPERTY(member)                                                                            \
    T##member;                                                                                          \
    enum:i32 { $##member##_id = i32(__COUNTER__) - $property_idx};                                      \
    auto    operator[](I32<$##member##_id>)        { return make_property(#member, member); }           \
    auto    operator[](I32<$##member##_id>) const  { return make_property(#member, member); }           \
    T##member member

#define _NMS_ENUM_CASE(i, value)  case TEnum::value: return StrView{#value};
#define _NMS_ENUM_IF(i,   value)  if (str == #value) { val = TEnum::value; return true; }

#define NMS_ENUM(type, ...)                                     \
enum class type{ __VA_ARGS__ };                                 \
static StrView str_cast(type value) {                           \
    using TEnum = type;                                         \
    switch(value) {                                             \
        NMSCPP_FOR(_NMS_ENUM_CASE, __VA_ARGS__)                 \
    }                                                           \
    return {};                                                  \
}                                                               \
static bool parse(StrView str, type& val, StrView fmt={}) {     \
    using TEnum = type;                                         \
    NMSCPP_FOR(_NMS_ENUM_IF, __VA_ARGS__)                       \
    return false;                                               \
}
