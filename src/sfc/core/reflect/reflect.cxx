
#include "sfc/test/test.h"

namespace sfc::reflect::test {

SFC_TEST(type_name) {
  static_assert(reflect::type_name<int>() == "int");
  static_assert(reflect::type_name<Str>() == "sfc::str::Str");
  static_assert(reflect::type_name<Option<int>>() == "sfc::option::Option<int>");
}

enum class E { A, B, C };
SFC_TEST(enum_name) {
  static_assert(reflect::enum_name<E::A>() == "A");
  static_assert(reflect::enum_name<E::B>() == "B");
  static_assert(reflect::enum_name<E::C>() == "C");
  static_assert(reflect::enum_name<static_cast<E>(3)>()._len == 0);

  static_assert(reflect::enum_name<static_cast<io::Error>(60)>()._len == 0);
}

}  // namespace sfc::reflect::test
