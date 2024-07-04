#include "sfc/test.h"

namespace sfc::fmt {

namespace {

enum class Enum1 {
  EA,
  EB,
};
SFC_ENUM(Enum1, EA, EB)

}  // namespace

SFC_TEST(enum) {
  test::assert_eq(string::format("{}", Enum1::EA), "EA");
  test::assert_eq(string::format("{}", Enum1::EB), "EB");
}

}  // namespace sfc::fmt
