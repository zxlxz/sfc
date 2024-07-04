#pragma once

#if defined(__clang__) && defined(_WIN32)
#pragma clang diagnostic ignored "-Wunknown-attributes"
#endif

namespace sfc {

using i8 = signed char;
using i16 = short;
using i32 = int;
using i64 = decltype(static_cast<char*>(0) - static_cast<char*>(0));

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = decltype(sizeof(0));

using isize = decltype(static_cast<char*>(0) - static_cast<char*>(0));
using usize = decltype(sizeof(0));

using f32 = float;
using f64 = double;

using cstr_t = const char*;

struct Dummy {};
static constexpr Dummy _ = {};

template <class T>
auto declval() -> T&&;

}  // namespace sfc

#if defined(_MSC_VER) && !defined(__clang__) // clang-format off
template <class T, class U> struct is_same       { static constexpr bool value = false;};
template <class T>          struct is_same<T, T> { static constexpr bool value = true; };
#define __is_same(T, U) is_same<T, U>::value
#endif // clang-format on
