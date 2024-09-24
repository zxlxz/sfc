#pragma once

#include "macro.h"
#include "type.h"

namespace sfc::reflect {

template <class T>
struct Field {
  str::Str name;
  T& value;
};

template <class T>
Field(str::Str, T&) -> Field<T>;

template <class T>
auto make_field(str::Str name, T& val) -> Field<T> {
  return {name, val};
}

template <class... U>
class StructInfo {
  using Fields = tuple::Tuple<Field<const U>...>;
  Fields _fields;

 public:
  StructInfo(Field<const U>... fields) : _fields{fields...} {}

  auto fields() const -> const Fields& {
    return _fields;
  }

  auto fields_mut() -> Fields& {
    return _fields;
  }
};

template <class... U>
StructInfo(Field<const U>...) -> StructInfo<U...>;

template <class T>
concept iReflect = requires(const T& x) { reflect_struct(x); };

template <class... U>
auto make_struct_info(Field<const U>... fields) -> StructInfo<U...> {
  return {fields...};
}

}  // namespace sfc::reflect

#define _SFC_STRUCT_FIELD(x) sfc::reflect::make_field(#x, self.x)
#define SFC_STRUCT(T, ...)                                                              \
  static inline auto reflect_struct(const T& self) {                                    \
    return sfc::reflect::make_struct_info(SFC_PP_LIST(_SFC_STRUCT_FIELD, __VA_ARGS__)); \
  }
