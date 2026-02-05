#include "sfc/test/test.h"
#include "sfc/core/option.h"

namespace sfc::option::test {

SFC_TEST(basic) {
  using Option = option::Option<int>;

  const auto none = Option{};
  panicking::expect_true(none.is_none());
  panicking::expect_false(none.is_some());

  const auto some = Option{10};
  panicking::expect_true(some.is_some());
  panicking::expect_false(some.is_none());
  panicking::expect_eq(*some, 10);
}

SFC_TEST(copy_ctor) {
  using Option = option::Option<int>;
  const auto none = Option{};
  const auto some = Option{10};

  const auto a = none;
  const auto b = some;
  panicking::expect_true(a.is_none());
  panicking::expect_true(b.is_some());
  panicking::expect_eq(*b, 10);
}

SFC_TEST(copy_assign) {
  using Option = option::Option<int>;
  const auto none = Option{};
  const auto some = Option{10};

  auto x = Option{};
  x = none;
  panicking::expect_true(x.is_none());

  x = some;
  panicking::expect_true(x.is_some());
  panicking::expect_eq(*x, 10);
}

SFC_TEST(move_ctor) {
  using Option = option::Option<String>;

  auto a = Option{};
  auto b = Option{String::from("hello")};

  auto x = mem::move(a);
  panicking::expect_true(x.is_none());

  auto y = mem::move(b);
  panicking::expect_true(y.is_some());
  panicking::expect_eq(*y, "hello");
}

SFC_TEST(move_assign) {
  using Option = option::Option<String>;

  auto a = Option{};
  auto b = Option{String::from("hello")};

  auto x = Option{};
  panicking::expect_true(x.is_none());

  x = mem::move(a);
  panicking::expect_true(x.is_none());

  x = mem::move(b);
  panicking::expect_true(x.is_some());
  panicking::expect_eq(*x, "hello");

  auto y = Option{String::from("world")};
  panicking::expect_eq(*y, "world");

  y = mem::move(x);
  panicking::expect_true(y.is_some());
  panicking::expect_eq(*y, "hello");
}

SFC_TEST(eq) {
  using Option = option::Option<int>;

  panicking::expect_eq(Option{}, Option{});
  panicking::expect_eq(Option{1}, Option{1});

  panicking::expect_ne(Option{}, Option{1});
  panicking::expect_ne(Option{1}, Option{2});
}

SFC_TEST(unwrap) {
  using Option = option::Option<int>;

  panicking::expect_eq(Option{}.unwrap_or(1), 1);
  panicking::expect_eq(Option{10}.unwrap(), 10);
  panicking::expect_eq(Option{10}.unwrap_or(5), 10);
}

SFC_TEST(and_or) {
  using Option = option::Option<int>;

  panicking::expect_eq(Option{} & Option{1}, Option{});
  panicking::expect_eq(Option{10} & Option{1}, Option{1});

  panicking::expect_eq(Option{} | Option{1}, Option{1});
  panicking::expect_eq(Option{10} | Option{1}, Option{10});
}

SFC_TEST(and_then) {
  using Option = option::Option<int>;
  auto add1 = [](auto x) { return Option{x + 1}; };

  panicking::expect_eq(Option{10}.and_then(add1), Option{11});
  panicking::expect_eq(Option{}.and_then(add1), Option{});
}

SFC_TEST(or_else) {
  using Option = option::Option<int>;
  auto make_opt = [] { return Option{1}; };

  panicking::expect_eq(Option{}.or_else(make_opt), Option{1});
  panicking::expect_eq(Option{10}.or_else(make_opt), Option{10});
}

SFC_TEST(map) {
  using Option = option::Option<int>;
  auto add1 = [](auto x) { return x + 1; };

  panicking::expect_eq(Option{}.map(add1), Option{});
  panicking::expect_eq(Option{10}.map(add1), Option{11});
}

SFC_TEST(map_or) {
  using Option = option::Option<int>;
  auto add1 = [](auto x) { return x + 1; };

  panicking::expect_eq(Option{}.map_or(5, add1), 5);
  panicking::expect_eq(Option{10}.map_or(5, add1), 11);
}

}  // namespace sfc::option::test
