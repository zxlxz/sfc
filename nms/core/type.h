#pragma once

#include <nms/core/base.h>
#include <nms/core/view.h>

namespace nms
{

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
        : get_name_(func_name)
    {}

#if defined(NMS_CC_MSVC)
    static constexpr auto funcsig_head_size_ = sizeof("struct nms::View<char const ,0> __cdecl nms::Type::_get_name<") - 1;
    static constexpr auto funcsig_tail_size_ = sizeof(">(void)") - 1;
#elif defined(NMS_CC_CLANG)
    static constexpr auto funcsig_head_size_ = sizeof("static View<const char> nms::Type::_get_name() [T = ") - 1;
    static constexpr auto funcsig_tail_size_ = sizeof("]") - 1;
#elif defined(NMS_CC_GNUC)
    static constexpr auto funcsig_head_size_ = sizeof("static nms::View<const char> nms::Type::_get_name() [with T = ") -1;
    static constexpr auto funcsig_tail_size_ = sizeof("]") -1;
#else
#   error("unknow c++ compiler")
#endif
    template<class T>
    static View<const char> _get_name() {
        const StrView name =  { __PRETTY_FUNCTION__ + funcsig_head_size_, { u32(sizeof(__PRETTY_FUNCTION__) - funcsig_head_size_ - funcsig_tail_size_ - 1) } };
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

}
