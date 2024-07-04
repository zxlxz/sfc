#include "sfc/test.h"

namespace sfc::reflect::detail {

SFC_TEST(type_name) {
  test::assert_eq(type_name<int>(), "int");
  test::assert_eq(type_name<sfc::Slice<int>>(), "sfc::slice::Slice<int>");
  test::assert_eq(type_name<sfc::Str>(), "sfc::str::Str");
}

}  // namespace sfc::reflect::detail
