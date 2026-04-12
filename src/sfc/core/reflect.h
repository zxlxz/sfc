#pragma once

#include "sfc/core/str.h"

namespace sfc::reflect {

template <class T>
consteval Str TYPE_NAME() {
#if defined(_MSC_VER) && !defined(__clang__)
  return Str{__FUNCSIG__, sizeof(__FUNCSIG__) - sizeof(">(void)")};
#else
  return Str{__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - sizeof("]")};
#endif
}

template <auto X>
consteval Str VALUE_NAME() {
#if defined(_MSC_VER) && !defined(__clang__)
  return Str{__FUNCSIG__, sizeof(__FUNCSIG__) - sizeof(">(void)")};
#else
  return Str{__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__) - sizeof("]")};
#endif
};

template <class T>
consteval Str type_name() {
  static constexpr auto Z = TYPE_NAME<void>();
  static constexpr auto S = TYPE_NAME<T>();
  static constexpr auto P = S._ptr + Z._len - (sizeof("void") - 1);
  static constexpr auto N = S._len - Z._len + (sizeof("void") - 1);
#if defined(_MSC_VER) && !defined(__clang__)
  if constexpr (P[5] == ' ') return Str{P + 6, N - 6};
  if constexpr (P[6] == ' ') return Str{P + 7, N - 7};
#endif
  return Str{P, N};
}

template <auto X>
consteval Str value_name() {
  static constexpr auto Z = VALUE_NAME<true>();
  static constexpr auto S = VALUE_NAME<X>();
  static constexpr auto P = S._ptr + Z._len - (sizeof("true") - 1);
  static constexpr auto N = S._len - Z._len + (sizeof("true") - 1);
  for (auto I = N; I != 0; --I) {
    if (P[I - 1] == ':') return Str{P + I, N - I};
  }
  return Str{P, N};
}

template <enum_ T, u32 I>
consteval auto enum_valid() -> bool {
  if constexpr (requires { static_cast<T>(I); }) {
    static constexpr auto E = static_cast<T>(I);
    return reflect::value_name<E>()._len != 0;
  }
  return false;
}

template <enum_ T, u32 I = 0, u32 N = 64>
consteval auto enum_count() -> u32 {
  if constexpr (I + 1 == N) {
    return N;
  } else if constexpr (reflect::enum_valid<T, I + (N - I) / 2>()) {
    return reflect::enum_count<T, I + (N - I) / 2, N>();
  } else {
    return reflect::enum_count<T, I, I + (N - I) / 2>();
  }
}

template <enum_ T>
constexpr auto enum_name(T val) -> Str {
#if defined(__clang__) || defined(__GNUC__)
  static constexpr auto IS_SCOPED_ENUM = __is_scoped_enum(T);
#else
  static constexpr auto IS_SCOPED_ENUM = !__is_convertible_to(T, int);
#endif
  if constexpr (IS_SCOPED_ENUM) {
    static constexpr auto N = reflect::enum_count<T>();
    static constexpr auto& names = []<auto... I>(idxs_t<I...>) -> auto& {
      static constexpr Str s[] = {reflect::value_name<static_cast<T>(I)>()..., {}};
      return s;
    }(sfc::seq_t<N>());
    const auto idx = static_cast<u32>(val);
    return idx < N ? names[idx] : Str{};
  }
  return {};
}

}  // namespace sfc::reflect
