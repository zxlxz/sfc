#pragma once

#include "sfc/core/str.h"

namespace sfc::reflect {

template <class T>
consteval Str type_name() {
  static constexpr auto S1 = sizeof("Str sfc::reflect::type_name() [T =");
  static constexpr auto S2 = sizeof("]");
  return Str{__PRETTY_FUNCTION__ + S1, sizeof(__PRETTY_FUNCTION__) - S1 - S2};
}

template <auto E>
consteval Str enum_name() {
  static constexpr auto S1 = sizeof("Str sfc::reflect::enum_name() [E =");
  static constexpr auto S2 = sizeof("]");
  static constexpr auto FN = __PRETTY_FUNCTION__ + S1;
  static constexpr auto SN = sizeof(__PRETTY_FUNCTION__) - S1 - S2;
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

template <trait::enum_ E, u32 I = 0, u32 N = 256>
consteval auto enum_count() -> u32 {
  if constexpr (I + 1 == N) {
    return N;
  } else if (!__is_scoped_enum(E)) {
    return 0;
  } else if constexpr (requires { reflect::enum_name<static_cast<E>(I)>(); }) {
    static constexpr auto name = reflect::enum_name<static_cast<E>(I)>();
    if constexpr (name._len != 0) {
      return reflect::enum_count<E, I + (N - I) / 2, N>();
    } else {
      return reflect::enum_count<E, I, I + (N - I) / 2>();
    }
  } else {
    return 0;
  }
}

template <trait::enum_ E>
consteval auto enum_names() -> Slice<const Str> {
  static constexpr auto N = reflect::enum_count<E>();
  if constexpr (N == 0) {
    return {};
  } else {
    static constexpr auto names = []<u32... I>(trait::idxs_t<I...>) {
      static constexpr Str s[N] = {reflect::enum_name<static_cast<E>(I)>()...};
      return Slice{s};
    }(trait::idxs_seq_t<N>());
    return names;
  }
}

template <trait::enum_ E>
constexpr auto to_str(E val) -> Str {
  static constexpr auto names = reflect::enum_names<E>();
  if (static_cast<u32>(val) >= names._len) {
    return {};
  }
  return names[static_cast<u32>(val)];
}

}  // namespace sfc::reflect
