#pragma once

#include "sfc/core/str.h"

namespace sfc::reflect {

template <class T>
consteval Str type_name() {
#if defined(__clang__) || defined(__GNUC__)
  // Str sfc::reflect::type_name() [T = $]
  static constexpr auto P = __PRETTY_FUNCTION__;
  static constexpr auto N = sizeof(__PRETTY_FUNCTION__) - 1;
  static constexpr auto A = sizeof("Str sfc::reflect::type_name() [T = ") - 1;
  static constexpr auto B = sizeof("sfc::str::Str sfc::reflect::type_name() [with T = ") - 1;
  static constexpr auto E = sizeof("]") - 1;
  static constexpr auto I = P[A - 1] == ' ' ? A : B;
#else
  // struct sfc::str::Str __cdecl sfc::reflect::type_name<$>(void)
  static constexpr auto P = __FUNCSIG__;
  static constexpr auto N = sizeof(__FUNCSIG__) - 1;
  static constexpr auto A = sizeof("struct sfc::str::Str __cdecl sfc::reflect::type_name<") - 1;
  static constexpr auto B = sizeof("struct sfc::str::Str __cdecl sfc::reflect::type_name<class ") - 1;
  static constexpr auto C = sizeof("struct sfc::str::Str __cdecl sfc::reflect::type_name<struct ") - 1;
  static constexpr auto E = sizeof(">(void)") - 1;
  static constexpr auto I = P[B - 1] == ' ' ? B : (P[C - 1] == ' ' ? C : A);
#endif
  return {P + I, N - I - E};
}

template <auto X>
consteval Str value_name() {
#if defined(__clang__) || defined(__GNUC__)
  // str::Str sfc::reflect::value_name() [with X = $]
  static constexpr auto P = __PRETTY_FUNCTION__;
  static constexpr auto N = sizeof(__PRETTY_FUNCTION__) - 1;
  static constexpr auto A = sizeof("str::Str sfc::reflect::value_name() [with X = ") - 1;
  static constexpr auto E = sizeof("]") - 1;
#else
  // struct sfc::str::Str __cdecl sfc::reflect::value_name<$>(void)
  static constexpr auto P = __FUNCSIG__;
  static constexpr auto N = sizeof(__FUNCSIG__) - 1;
  static constexpr auto A = sizeof("struct sfc::str::Str __cdecl sfc::reflect::value_name<") - 1;
  static constexpr auto E = sizeof(">(void)") - 1;
#endif
  for (auto I = N - E; I != A; --I) {
    if (P[I - 1] == ':') return {P + I, N - I - E};
  }
  return {P + A, N - A - E};
}

template <enum_ T, u32 I>
consteval auto enum_valid() -> bool {
  if constexpr (!__is_scoped_enum(T)) {
    return false;
  } else if constexpr (requires { static_cast<T>(I); }) {
    static constexpr auto E = static_cast<T>(I);
    return reflect::value_name<E>()._len != 0;
  }
  return false;
}

template <enum_ T, u32 I = 0, u32 N = 64>
consteval auto enum_count() -> u32 {
  if constexpr (I + 1 == N) {
    return N;
  }
  if constexpr (reflect::enum_valid<T, I + (N - I) / 2>()) {
    return reflect::enum_count<T, I + (N - I) / 2, N>();
  } else {
    return reflect::enum_count<T, I, I + (N - I) / 2>();
  }
}

template <enum_ T>
constexpr auto enum_name(T val) -> Str {
  static constexpr auto N = reflect::enum_count<T>();
  static constexpr auto& names = []<auto... I>(idxs_t<I...>) -> auto& {
    static constexpr Str s[] = {reflect::value_name<static_cast<T>(I)>()..., {}};
    return s;
  }(seq_t<N>());

  if (__is_scoped_enum(T)) {
    const auto idx = static_cast<u32>(val);
    return idx < N ? names[idx] : Str{};
  }
  return {};
}

}  // namespace sfc::reflect
