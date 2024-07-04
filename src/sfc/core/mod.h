#pragma once

#define sfc_inline gnu::always_inline

#if !defined(__clang__) && defined(__GNUC__)

#if __GNUC__ <= 6
#define concept concept bool
#pragma GCC diagnostic ignored "-Wattributes"
#endif

#if __GNUC__ <= 9
#define __is_same __is_same_as
#endif
#endif

namespace sfc {

using i8 = __INT8_TYPE__;
using i16 = __INT16_TYPE__;
using i32 = __INT32_TYPE__;
using i64 = __INT64_TYPE__;

using u8 = __UINT8_TYPE__;
using u16 = __UINT16_TYPE__;
using u32 = __UINT32_TYPE__;
using u64 = __UINT64_TYPE__;

using isize = __INTPTR_TYPE__;
using usize = __SIZE_TYPE__;

using f32 = float;
using f64 = double;

using cstr_t = const char*;

struct Dummy {};
static constexpr Dummy _ = {};

template <class T>
auto declval() noexcept -> T&&;

}  // namespace sfc
