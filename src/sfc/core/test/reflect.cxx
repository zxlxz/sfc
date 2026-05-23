#include "sfc/test/test.h"
#include "sfc/core/reflect.h"

namespace sfc::reflect::test {

SFC_TEST(type_name) {
  static_assert(reflect::type_name<int>() == "int");
  static_assert(reflect::type_name<Str>() == "sfc::str::Str");
  static_assert(reflect::type_name<Option<int>>() == "sfc::option::Option<int>");
}

enum class E { A, B, C };
SFC_TEST(value_name) {
  static_assert(reflect::value_name<E::A>() == "A");
  static_assert(reflect::value_name<E::B>() == "B");
  static_assert(reflect::value_name<E::C>() == "C");
}

}  // namespace sfc::reflect::test
