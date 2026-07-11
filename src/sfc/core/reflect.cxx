#include "sfc/test/test.h"
#include "sfc/core/reflect.h"

namespace sfc::reflect::test {

SFC_TEST(type_name) {
  sfc::assert_eq(reflect::type_name<int>(), "int");
  sfc::assert_eq(reflect::type_name<Str>(), "sfc::str::Str");
  sfc::assert_eq(reflect::type_name<Option<int>>(), "sfc::option::Option<int>");
}

enum class E { A, B, C };
SFC_TEST(value_name) {
  sfc::assert_eq(reflect::value_name<E::A>(), "A");
  sfc::assert_eq(reflect::value_name<E::B>(), "B");
  sfc::assert_eq(reflect::value_name<E::C>(), "C");
}

}  // namespace sfc::reflect::test
