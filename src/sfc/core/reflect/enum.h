#pragma once

#include "macro.h"
#include "type.h"

namespace sfc::reflect {

template <class T>
struct Enum {
  cstr_t name;
  T value;
};

#if __cplusplus >= 202002L
template <class T>
Enum(cstr_t, T) -> Enum<T>;
#endif

template <class T>
auto make_enum(cstr_t name, T value) -> Enum<T> {
  return {name, value};
}

template <class T, usize N>
struct EnumInfo {
  using Item = Enum<T>;
  Item _items[N];

 public:
  auto get_name_by_value(T val) const -> cstr_t {
    for (const auto& item : _items) {
      if (item.value == val) {
        return item.name;
      }
    }
    return nullptr;
  }

  auto get_value_by_name(str::Str name) const -> Option<T> {
    for (const auto& item : _items) {
      if (name == str::Str{item.name}) {
        return item.value;
      }
    }
    return {};
  }
};

#if __cplusplus >= 202002L
template <class T, class... U>
EnumInfo(const Enum<T>&, const Enum<U>&...) -> EnumInfo<T, 1 + sizeof...(U)>;
#endif

template <class T>
auto make_enum_info(const Enum<T>& t, const auto&... u) -> EnumInfo<T, 1 + sizeof...(u)> {
  return {t, u...};
};

template <class T>
auto enum_name(T val) -> cstr_t {
  const auto info = reflect_enum(static_cast<T>(0));
  return info.get_name_by_value(val);
}

template <class T>
auto enum_from_name(Str name) -> Option<T> {
  const auto info = reflect_enum(static_cast<T>(0));
  return info.get_value_by_name(name);
}

}  // namespace sfc::reflect

// clang-format off
#define _SFC_ENUM_ENUM(x) sfc::reflect::make_enum(#x, _ET::x)
#define SFC_ENUM(T, ...)                                                      \
  inline auto reflect_enum(T) {                                               \
    using _ET = T;                                                            \
    return sfc::reflect::make_enum_info(SFC_PP_LIST(_SFC_ENUM_ENUM, __VA_ARGS__));  \
  }
// clang-format on
