#pragma once

#if defined(__clang__) && defined(_WIN32)
#pragma clang diagnostic ignored "-Wunknown-attributes"
#endif

// clang-format off
#if defined(__clang__) && defined(__APPLE__)
#define SFC_STD std::inline __1
#else
#define SFC_STD std
#endif
// clang-format on

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

template <class T>
struct Expr {
  using Type = T;
};

template <class F, class... T>
struct Expr<F(T...)> {
  using Type = decltype(declval<F>()(declval<T>()...));
};

template <class X>
using expr_t = typename Expr<X>::Type;


}  // namespace sfc
