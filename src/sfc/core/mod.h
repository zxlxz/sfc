#pragma once

#ifdef __clang__
#pragma clang diagnostic ignored "-Wc++26-extensions"
#pragma clang diagnostic ignored "-Wgnu-statement-expression"
#endif

#if defined __clang__ && defined(_WIN32)
#define no_unique_address msvc::no_unique_address
#endif

namespace sfc {

using i8 = signed char;
using i16 = short;
using i32 = int;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

#ifdef _WIN32
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

template <class T>
concept enum_ = __is_enum(T);

template <class T>
concept class_ = __is_class(T);

#if defined(__GNUC__) || defined(__clang__)
template <class T, class U>
concept same_ = __is_same(T, U);
#else
template <class T, class U>
struct same_t {
  static constexpr bool value = false;
};
template <class T>
struct same_t<T, T> {
  static constexpr bool value = true;
};
template <class T, class U>
concept same_ = same_t<T, U>::value;
#endif

template <auto... I>
struct idxs_t {};

#if defined(__GNUC__) && !defined(__clang__)
template <auto N>
using idxs_seq_t = idxs_t<__integer_pack(N)...>;
#else
template <class, auto... I>
struct _IntSeq {
  using Type = idxs_t<I...>;
};

template <auto N>
using seq_t = typename __make_integer_seq<_IntSeq, decltype(N), N>::Type;
#endif

namespace str {
struct Str;
}

namespace fmt {
struct RawStr;
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
