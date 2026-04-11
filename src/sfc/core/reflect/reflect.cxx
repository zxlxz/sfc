
#include "sfc/core/reflect.h"

namespace sfc::reflect::test {

static void test_name() {
  static_assert(reflect::type_name<int>() == "int");
  static_assert(reflect::type_name<Str>() == "sfc::str::Str");
  static_assert(reflect::type_name<Option<int>>() == "sfc::option::Option<int>");
}

enum class E { A, B, C };
static void test_enum() {
  static_assert(reflect::enum_name<E::A>() == "A");
  static_assert(reflect::enum_name<E::B>() == "B");
  static_assert(reflect::enum_name<E::C>() == "C");
}

}  // namespace sfc::reflect::test
