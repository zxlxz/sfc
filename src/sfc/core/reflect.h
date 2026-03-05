#pragma once

#include "sfc/core/str.h"

namespace sfc::reflect {

template <class T>
consteval Str type_name() {
  static constexpr auto FN = __PRETTY_FUNCTION__;
  static constexpr auto SN = sizeof(__PRETTY_FUNCTION__) - 2;
  for (auto i = 0UL; i < SN - 2; ++i) {
    if (FN[i] == '=') {
      return Str{FN + i + 2, SN - i - 2};
    }
  }
  return Str{__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__)};
}

template <auto E>
consteval Str enum_name() {
  static constexpr auto FN = __PRETTY_FUNCTION__;
  static constexpr auto SN = sizeof(__PRETTY_FUNCTION__) - 2;
  for (auto i = SN - 2; i != 0; --i) {
    if (FN[i] == ':' || FN[i] == ' ') {
      return Str{FN + i + 1, SN - i - 1};
    }
    if (FN[i] == ')') {
      return {};
    }
  }
  return Str{__PRETTY_FUNCTION__, sizeof(__PRETTY_FUNCTION__)};
}

template <trait::enum_ E, u32 I = 0, u32 N = 64>
consteval auto enum_count() -> u32 {
  if constexpr (I + 1 == N) {
    return N;
  } else if constexpr (__is_scoped_enum(E) && requires { reflect::enum_name<static_cast<E>(I)>(); }) {
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

template <trait::enum_ E, u32 N = enum_count<E>()>
consteval auto enum_names() -> Slice<const Str> {
  if constexpr (N == 0 || !__is_scoped_enum(E)) {
    return {};
  } else {
    static constexpr auto names = []<auto... I>(trait::idxs_t<I...>) {
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
