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
consteval Str enum_name() {
#if defined(__clang__) || defined(__GNUC__)
  // str::Str sfc::reflect::enum_name() [with X = $]
  static constexpr auto P = __PRETTY_FUNCTION__;
  static constexpr auto N = sizeof(__PRETTY_FUNCTION__) - 1;
  static constexpr auto A = sizeof("str::Str sfc::reflect::enum_name() [with X = ") - 1;
  static constexpr auto E = sizeof("]") - 1;
#else
  // struct sfc::str::Str __cdecl sfc::reflect::enum_name<$>(void)
  static constexpr auto P = __FUNCSIG__;
  static constexpr auto N = sizeof(__FUNCSIG__) - 1;
  static constexpr auto A = sizeof("struct sfc::str::Str __cdecl sfc::reflect::enum_name<") - 1;
  static constexpr auto E = sizeof(">(void)") - 1;
#endif
  for (auto I = N - E; I != A; --I) {
    if (P[I - 1] == ':') return {P + I, N - I - E};
  }
  return {P + A, N - A - E};
}

template <enum_ E, u32 I = 0, u32 N = 64>
consteval auto enum_count() -> u32 {
  if constexpr (I + 1 == N) {
    return N;
  } else if constexpr (requires { reflect::enum_name<static_cast<E>(I)>(); }) {
    static constexpr auto name = reflect::enum_name<static_cast<E>(I)>();
    if constexpr (name._len == 0) {
      return reflect::enum_count<E, I, I + (N - I) / 2>();
    } else {
      return reflect::enum_count<E, I + (N - I) / 2, N>();
    }
  } else {
    return 0;
  }
}

template <enum_ E, u32 N = enum_count<E>()>
consteval auto enum_names() -> Slice<const Str> {
  if constexpr (N == 0) {
    return {};
  } else {
    static constexpr auto names = []<auto... I>(idxs_t<I...>) {
      static constexpr Str s[N] = {reflect::enum_name<static_cast<E>(I)>()...};
      return Slice{s};
    }(seq_t<N>());
    return names;
  }
}

template <enum_ E>
constexpr auto to_str(E val) -> Str {
  static constexpr auto names = reflect::enum_names<E>();
  if (static_cast<u32>(val) >= names._len) {
    return {};
  }
  return names[static_cast<u32>(val)];
}

}  // namespace sfc::reflect
