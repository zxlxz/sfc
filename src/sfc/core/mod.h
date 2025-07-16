#pragma once

namespace sfc {

using i8  = signed char;
using i16 = short;
using i32 = int;
using i64 = decltype(static_cast<char*>(0) - static_cast<char*>(0));

using u8  = unsigned char;
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

#if !defined(_NEW) && !defined(_LIBCPP_NEW) && !defined(__PLACEMENT_NEW_INLINE)
inline auto operator new(sfc::usize size, void* ptr) noexcept -> void* {
  (void)size;
  return ptr;
}
#endif
