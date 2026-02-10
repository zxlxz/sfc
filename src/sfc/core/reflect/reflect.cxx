
#include "sfc/test/test.h"

namespace sfc::reflect::test {

SFC_TEST(type_name) {
  static_assert(reflect::type_name<int>() == "int");
  static_assert(reflect::type_name<Str>() == "sfc::str::Str");
  static_assert(reflect::type_name<Option<int>>() == "sfc::option::Option<int>");
}

enum E1 { A, B };
enum class E2 { A, B };
SFC_TEST(enum_name) {
  static_assert(reflect::enum_name<E1::A>() == "A");
  static_assert(reflect::enum_name<E2::B>() == "B");
}

SFC_TEST(enum_valid) {
  static_assert(reflect::enum_valid<E1, 0>());
  static_assert(reflect::enum_valid<E1, 1>());
}

}  // namespace sfc::reflect::test
