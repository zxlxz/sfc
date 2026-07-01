#pragma once

#include "sfc/core/mod.h"

namespace sfc::trait {

template <class T>
struct type_t {};

template <auto X>
struct const_t {
  static constexpr auto VALUE = X;
};

template <class T>
concept enum_ = __is_enum(T);

template <class T>
concept class_ = __is_class(T);

#if defined(__clang__) || defined(__GNUC__)
template <class T, class U>
concept same_ = __is_same(T, U);
#else
template <class T, class U>
concept same_ = __is_convertible_to(type_t<T>*, type_t<U>*);
#endif

template <class T, class... U>
concept any_ = (same_<T, U> || ...);

template <class T>
concept sint_ = any_<T, signed char, short, int, long, long long>;

template <class T>
concept uint_ = any_<T, unsigned char, unsigned short, unsigned int, unsigned long, unsigned long long>;

template <class T>
concept int_ = sint_<T> || uint_<T>;

template <class T>
concept float_ = any_<T, float, double>;

template <class T>
concept tv_copy_ = __is_trivially_copyable(T);

template <class T>
concept tv_drop_ = __is_trivially_destructible(T);

template <class T, class... U>
concept AsRef = requires(const U&... args) { T{args...}; };

}  // namespace sfc::trait

namespace sfc {
using trait::same_;
using trait::enum_;
using trait::class_;

using trait::int_;
using trait::sint_;
using trait::uint_;
using trait::float_;
}  // namespace sfc
