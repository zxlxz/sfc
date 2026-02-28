#pragma once

#include "sfc/core/str.h"
namespace sfc::reflect {

template <class T>
consteval Str type_name() {
#if defined(_MSC_VER) && !defined(__clang__)
  static constexpr char FN[] = __FUNCSIG__;
  static constexpr auto S1 = sizeof("sfc::str::Str sfc::reflect::type_name() [with T =");
  static constexpr auto S2 = sizeof("]");
  return Str{__FUNCSIG__ + S1, sizeof(__FUNCSIG__) - S1 - S2};
#else
  static constexpr auto S1 = sizeof("Str sfc::reflect::type_name() [T =");
  static constexpr auto S2 = sizeof("]");
  return Str{__PRETTY_FUNCTION__ + S1, sizeof(__PRETTY_FUNCTION__) - S1 - S2};
#endif
}

template <auto E>
consteval Str enum_name() {
#if defined(_MSC_VER) && !defined(__clang__)
  static constexpr auto S1 = sizeof("sfc::str::Str sfc::reflect::enum_name() [with E =");
  static constexpr auto S2 = sizeof("]");
  static constexpr auto FN = __FUNCSIG__ + S1;
  static constexpr auto SN = sizeof(__FUNCSIG__) - S1 - S2;
#else
  static constexpr auto S1 = sizeof("Str sfc::reflect::enum_name() [E =");
  static constexpr auto S2 = sizeof("]");
  static constexpr auto FN = __PRETTY_FUNCTION__ + S1;
  static constexpr auto SN = sizeof(__PRETTY_FUNCTION__) - S1 - S2;
#endif
  for (auto i = SN; i != 0; --i) {
    if (FN[i - 1] == ')') {
      return {};
    }
    if (FN[i - 1] == ':') {
      return Str{FN + i, SN - i};
    }
  }
  return Str{FN, SN};
}

template <class E, auto I>
consteval auto enum_valid() -> bool {
  if constexpr (requires { static_cast<E>(I); }) {
    if constexpr (requires { reflect::enum_name<static_cast<E>(I)>(); }) {
      return reflect::enum_name<static_cast<E>(I)>()._len != 0;
    }
  }
  return false;
}

template <class E, u32 I = 0, u32 N = 256>
consteval auto enum_count() -> u32 {
  if constexpr (I + 1 == N) {
    return N;
  } else if constexpr (reflect::enum_valid<E, I + (N - I) / 2>()) {
    return reflect::enum_count<E, I + (N - I) / 2, N>();
  } else {
    return reflect::enum_count<E, I, I + (N - I) / 2>();
  }
}

template <trait::enum_ E>
constexpr auto to_str(E val) -> Str {
  static constexpr auto N = reflect::enum_count<E>();
  static Str names[N] = {};
  static const auto _init = []<u32... I>(trait::idxs_t<I...>) {
    ((names[I] = reflect::enum_name<static_cast<E>(I)>()), ...);
    return true;
  }(trait::idxs_seq_t<N>());

  const auto s = static_cast<u32>(val) < N ? names[static_cast<u32>(val)] : Str{};
  return s;
}

}  // namespace sfc::reflect
