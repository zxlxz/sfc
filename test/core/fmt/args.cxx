#include "sfc/collections.h"
#include "sfc/test.h"

namespace sfc::fmt::test {

SFC_TEST(list) {
  u32 v[] = {1, 2, 3};
  panicking::assert_eq(string::format("{}", v), "[1, 2, 3]");
}

SFC_TEST(dict) {
  auto m = VecMap<Str, int>{};
  m.insert("1", -1);
  m.insert("2", -2);
  panicking::assert_eq(string::format("{}", m), R"({"1": -1, "2": -2})");
}

struct T1 {
  int x;
  int y;

  void fmt(auto& f) const {
    f.debug_struct().field("x", x).field("y", y);
  }
};

SFC_TEST(object) {
  auto o = T1{1, 2};
  panicking::assert_eq(string::format("{}", o), R"({x: 1, y: 2})");
}

SFC_TEST(args) {
  panicking::assert_eq(string::format("`{}`", 1), "`1`");
  panicking::assert_eq(string::format("`{},{}`", 1, 2), "`1,2`");
}

}  // namespace sfc::fmt::test
