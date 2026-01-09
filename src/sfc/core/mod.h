#pragma once

#ifdef __clang__
#pragma clang diagnostic ignored "-Wc++26-extensions"
#pragma clang diagnostic ignored "-Wunknown-attributes"
#pragma clang diagnostic ignored "-Wgnu-statement-expression-from-macro-expansion"
#endif


namespace sfc {

using i8 = signed char;
using i16 = short;
using i32 = int;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

#ifdef _MSC_VER
using i64 = long long;
using u64 = unsigned long long;
#else
using i64 = long;
using u64 = unsigned long;
#endif

using isize = decltype(static_cast<char*>(0) - static_cast<char*>(0));
using usize = decltype(sizeof(0));

using f32 = float;
using f64 = double;

using cstr_t = const char*;

namespace str {
struct Str;
}

namespace slice {
template <class T>
struct Slice;
}

namespace option {
template <class T>
class Option;
}

namespace result {
template <class T, class E>
class Result;
}

}  // namespace sfc

#ifdef __APPLE__
#define SFC_STD std::inline __1
#else
#define SFC_STD std
#endif
