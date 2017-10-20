#pragma once

#include <ustd/core/view.h>

namespace ustd
{

using str  = View<const char>;
using strs = View<str>;

template<class T>
fn type_name() -> str {

#if defined(_MSC_VER)
    let prefix_len = sizeof("struct ustd::slice<char const, 0> __cdecl ustd::type_name<struct ") - 1;
    let suffix_len = sizeof("::_>(void)") - 1;
    return { __FUNCSIG__ + prefix_len, u32(sizeof(__FUNCSIG__) - prefix_len - suffix_len - 1) };
#else
    let prefix_len = sizeof("str ustd::type_name() [T = ") - 1;
    let suffix_len = sizeof("::_]") - 1;
    return { __PRETTY_FUNCTION__ + prefix_len, u32(sizeof(__PRETTY_FUNCTION__) - prefix_len - suffix_len - 1) };
#endif

}

}
