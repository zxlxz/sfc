#pragma once

#include <nms/core/base.h>
#include <nms/core/cpp.h>
#include <nms/core/view.h>

namespace nms
{

#pragma region type
struct Type
{
    __forceinline StrView name() const {
        return get_name_();
    }

    template<class T>
    constexpr static Type make() {
        return Type{ _get_name<T> };
    }

    friend __forceinline constexpr bool operator==(Type a, Type b) {
        return a.get_name_ == b.get_name_;
    }

    friend __forceinline constexpr bool operator!=(Type a, Type b) {
        return a.get_name_ != b.get_name_;
    }

private:
    StrView(*get_name_)();

    explicit constexpr Type(StrView(*func_name)())
        : get_name_(func_name) {}

#if defined(NMS_CC_MSVC)
    static constexpr auto funcsig_head_size_ = sizeof("struct nms::View<char const ,0> __cdecl nms::Type::_get_name<") - 1;
    static constexpr auto funcsig_tail_size_ = sizeof(">(void)") - 1;
#elif defined(NMS_CC_CLANG)
    static constexpr auto funcsig_head_size_ = sizeof("static View<const char> nms::Type::_get_name() [T = ") - 1;
    static constexpr auto funcsig_tail_size_ = sizeof("]") - 1;
#elif defined(NMS_CC_GNUC)
    static constexpr auto funcsig_head_size_ = sizeof("static nms::View<const char> nms::Type::_get_name() [with T = ") - 1;
    static constexpr auto funcsig_tail_size_ = sizeof("]") - 1;
#else
#   error("unknow c++ compiler")
#endif
    template<class T>
    static View<const char> _get_name() {
        const StrView name = { __PRETTY_FUNCTION__ + funcsig_head_size_, { u32(sizeof(__PRETTY_FUNCTION__) - funcsig_head_size_ - funcsig_tail_size_ - 1) } };
        return name;
    }
};

template<class T>
__forceinline constexpr Type typeof() {
    return Type::make<T>();
}

template<class T>
__forceinline constexpr Type typeof(const T&) {
    return Type::make<T>();
}
#pragma endregion

#pragma region enum
template<class T>
struct Enum
{
    T value;

    StrView name() const {
        const auto& names = enum_names(static_cast<T>(0));
        const auto  idx = static_cast<u32>(value);
        if (idx >= names.count()) {
            return {};
        }

        const auto str = names[idx];
        return str;
    }

    static T parse(StrView str) {
        const auto& names = enum_names(static_cast<T>(0));
        const auto  cnt = names.count();

        for (u32 i = 0; i < cnt; ++i) {
            auto name = names[i];
            if (name == str) {
                return static_cast<T>(i);
            }
        }
        return static_cast<T>(0);
    }
};

template<class T>
__forceinline constexpr Enum<T> mkEnum(const T& t) {
    return { t };
}

template<u32 N>
__forceinline StrView mkEnumName(const char(&s)[N]) {
    if (s[0] == '$') {
        return StrView{ s + 1, {N - 2} };
    }
    return { s, {N-1} };
}

#define _NMS_ENUM_NAME(value)  nms::mkEnumName(#value),

#define NMS_ENUM(type, ...)                                                                 \
enum class type { __VA_ARGS__};                                                             \
inline auto enum_names(type) {                                                              \
    static StrView names[] = { NMSCPP_FOR(_NMS_ENUM_NAME, __VA_ARGS__)};                    \
    return View<const StrView>{names, { u32(sizeof(names) / sizeof(names[0])) }};           \
}

#define NMS_ENUM_EX(enum_decl, type, ...)                                                   \
enum_decl { __VA_ARGS__};                                                                   \
inline auto enum_names(type) {                                                              \
    static StrView names[] = { NMSCPP_FOR(_NMS_ENUM_NAME, __VA_ARGS__)};                    \
    return View<const StrView>{names, { u32(sizeof(names) / sizeof(names[0])) }};           \
}

#pragma endregion

#pragma region property
template<class T>
struct Property
{
    StrView name;
    T       value;

    constexpr Property(StrView name, T value)
        : name(name), value(value)
    {}
};

template<class T, u32 N>
Property<T&> make_property(const char(&name)[N], T& member) {
    return { name, member };
}

template<class T, u32 N>
Property<const T&> make_property(const char(&name)[N], const T& member) {
    return { name, member };
}

#define NMS_PROPERTY_BEGIN enum:i32 { _$property_idx = __COUNTER__ + 1 }
#define NMS_PROPERTY_END   enum:i32 { _$property_cnt = __COUNTER__ - _$property_idx}

#define NMS_PROPERTY_(id, member, name)                                                             \
    T##member;                                                                                      \
    enum:i32 { _$##member##_id = i32(id) - _$property_idx};                                         \
    auto    operator[](I32<_$##member##_id>)        { return nms::make_property(name, member); }    \
    auto    operator[](I32<_$##member##_id>) const  { return nms::make_property(name, member); }    \
    T##member member

#define NMS_PROPERTY_1(id, member, ____)    NMS_PROPERTY_(id, member, #member)
#define NMS_PROPERTY_2(id, member, name)    NMS_PROPERTY_(id, member, name)

#define NMS_PROPERTY_$(id, x, y, n,...)     NMS_PROPERTY_##n(id, x, y)
#define NMS_PROPERTY_X(...)                 NMS_PROPERTY_$ __VA_ARGS__
#define NMS_PROPERTY(...)                   NMS_PROPERTY_X((__COUNTER__, __VA_ARGS__, 2, 1, ~))

#pragma endregion

}
