#pragma once

#include "macro.h"
#include "type.h"

namespace sfc::reflect {

template <class T>
struct Enum {
  cstr_t name;
  T value;
};

template <class T, usize N>
struct EnumInfo {
  Enum<T> _items[N];

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

#define _SFC_ENUM_ENUM(x) {#x, Self::x}
#define SFC_ENUM(T, ...)                                       \
  inline auto reflect_enum(T) {                                \
    using Self = T;                                            \
    return sfc::reflect::EnumInfo<T, SFC_PP_CNT(__VA_ARGS__)>{ \
        {SFC_PP_LIST(_SFC_ENUM_ENUM, __VA_ARGS__)}};           \
  }
