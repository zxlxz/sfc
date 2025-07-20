#include "sfc/test.h"

namespace sfc::fmt::test {

enum class E1 {
  EA,
  EB,
};

SFC_TEST(enum) {
  //panicking::assert_eq(string::format("{}", Enum1::EA), "EA");
  //panicking::assert_eq(string::format("{}", Enum1::EB), "EB");
}

}  // namespace sfc::fmt
