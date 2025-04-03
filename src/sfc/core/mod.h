#pragma once

#define no_unique_address

namespace sfc {

using i8  = signed char;
using i16 = short;
using i32 = int;
using i64 = decltype(10000000000);

using u8  = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = decltype(10000000000U);

using isize = decltype((char*)0 - (char*)0);
using usize = decltype(sizeof(0));

using f32 = float;
using f64 = double;

using cstr_t = const char*;

struct Dummy {};
static constexpr Dummy _ = {};

template <class T>
auto declval() -> T&&;

}  // namespace sfc
