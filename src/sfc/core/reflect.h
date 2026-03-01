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

template <trait::enum_ E, auto I>
constexpr auto enum_valid() -> bool {
  if constexpr (requires { static_cast<E>(I); }) {
    if constexpr (requires { reflect::enum_name<static_cast<E>(I)>(); }) {
      return reflect::enum_name<static_cast<E>(I)>()._len != 0;
    }
  }
  return false;
}

template <trait::enum_ E, u32 I = 0, u32 N = 256>
constexpr auto enum_count() -> u32 {
  static_assert(__is_scoped_enum(E));
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
  if constexpr (__is_scoped_enum(E)) {
    static constexpr auto N = reflect::enum_count<E>();
    static const auto& names = []<u32... I>(trait::idxs_t<I...>) {
      static const Str s[N] = {reflect::enum_name<static_cast<E>(I)>()...};
      return s;
    }(trait::idxs_seq_t<N>());
    if (static_cast<u32>(val) < N) {
      return names[static_cast<u32>(val)];
    }
  }
  return {};
}

}  // namespace sfc::reflect
