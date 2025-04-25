#pragma once

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
auto declval() -> T&&;

}  // namespace sfc

#if !defined(__PLACEMENT_NEW_INLINE) && !defined(_NEW)
inline auto operator new(__SIZE_TYPE__ size, void* ptr) noexcept -> void* {
  (void)size;
  return ptr;
}
#endif
