#pragma once

#include "sfc/core/str.h"

namespace sfc::reflect {

template <class T>
auto type_id() -> usize {
  static const int id = 0;
  return reinterpret_cast<usize>(&id);
}

template <class T>
auto type_name() -> str::Str {
#if defined(__clang__)
  const auto PREFIX = sizeof("str::Str sfc::reflect::type_name() [T = ") - 1;
  const auto SUFFIX = sizeof("]") - 1;
#elif defined(__GNUC__)
  const auto PREFIX = sizeof("sfc::str::Str sfc::reflect::type_name() [with T = ") - 1;
  const auto SUFFIX = sizeof("]") - 1;
#else
  const auto PREFIX = 0U;
  const auto SUFFIX = 0U;
#endif
  const auto fun = __PRETTY_FUNCTION__;
  const auto len = sizeof(__PRETTY_FUNCTION__) - 1 - PREFIX - SUFFIX;
  return str::Str{fun + PREFIX, len};
}

}  // namespace sfc::reflect
