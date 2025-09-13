#pragma once

#ifdef __clang__
#pragma clang diagnostic ignored "-Wc++26-extensions"
#pragma clang diagnostic ignored "-Wunknown-attributes"
#endif

#define SFC_STD std

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

}  // namespace sfc

#if !defined(__PLACEMENT_NEW_INLINE) && !defined(_LIBCPP_NEW)
inline void* operator new(sfc::usize, void* ptr) noexcept {
  return ptr;
}
inline void operator delete(void*, void*) noexcept {
  return;
}
#endif
