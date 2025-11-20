#include "sfc/test.h"
#include "sfc/core/option.h"

namespace sfc::option::test {

SFC_TEST(simple) {
  {
    const auto opt = Option<int>{};
    panicking::expect_false(opt.is_some());
    panicking::expect_true(opt.is_none());
  }

  {
    const auto opt = Option<int>{10};
    panicking::expect_true(opt.is_some());
    panicking::expect_false(opt.is_none());

    panicking::expect_eq(*opt, 10);
  }
}

SFC_TEST(eq) {
  const auto o0 = Option<int>{};
  const auto o1 = Option<int>{1};
  const auto o2 = Option<int>{2};

  panicking::expect_eq(o0, o0);
  panicking::expect_eq(o1, o1);

  panicking::expect_ne(o0, o1);
  panicking::expect_ne(o1, o2);

  panicking::expect_eq(*o1, 1);
  panicking::expect_eq(*o2, 2);
}

SFC_TEST(unwrap) {
  panicking::expect_eq(Option<int>{}.unwrap_or(1), 1);

  panicking::expect_eq(Option<int>{10}.unwrap(), 10);
  panicking::expect_eq(Option<int>{10}.unwrap_or(5), 10);
}

SFC_TEST(and) {
  panicking::expect_eq(Option<int>{} & Option{1}, Option<int>{});
  panicking::expect_eq(Option<int>{10} & Option{1}, Option<int>{1});
}

SFC_TEST(or) {
  panicking::expect_eq(Option<int>{} | Option{1}, Option{1});
  panicking::expect_eq(Option<int>{10} | Option{1}, Option{10});
}

SFC_TEST(and_then) {
  auto add = [](auto val) { return [val](auto x) { return Option{x + val}; }; };

  panicking::expect_eq(Option<int>{10}.and_then(add(1)), Option{11});
  panicking::expect_eq(Option<int>{}.and_then(add(1)), Option<int>{});
}

SFC_TEST(or_else) {
  auto make_opt = [](auto val) { return [val]() { return Option{val}; }; };
  panicking::expect_eq(Option<int>{}.or_else(make_opt(1)), Option{1});
  panicking::expect_eq(Option<int>{10}.or_else(make_opt(1)), Option{10});
}

SFC_TEST(map) {
  auto add = [](auto val) { return [val](auto x) { return x + val; }; };
  panicking::expect_eq(Option<int>{}.map(add(1)), Option<int>{});
  panicking::expect_eq(Option<int>{10}.map(add(1)), Option{11});
}

SFC_TEST(map_or) {
  auto add = [](auto val) { return [val](auto x) { return x + val; }; };
  panicking::expect_eq(Option<int>{}.map_or(5, add(1)), 5);
  panicking::expect_eq(Option<int>{10}.map_or(5, add(1)), 11);
}

}  // namespace sfc::option::test
