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


}  // namespace sfc::reflect
