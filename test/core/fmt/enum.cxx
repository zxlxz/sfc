#include "sfc/test.h"

namespace sfc::fmt {

enum class E1 {
  EA,
  EB,
};

SFC_TEST(enum) {
  //test::assert_eq(string::format("{}", Enum1::EA), "EA");
  //test::assert_eq(string::format("{}", Enum1::EB), "EB");
}

}  // namespace sfc::fmt
