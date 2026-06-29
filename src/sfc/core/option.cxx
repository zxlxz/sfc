#include "sfc/test/test.h"
#include "sfc/core/option.h"

namespace sfc::option::test {

SFC_TEST(basic) {
  const auto none = Option{};
  sfc::assert_eq(none.is_none(), true);
  sfc::assert_eq(none.is_some(), false);

  const auto some = Option{10};
  sfc::assert_eq(some.is_some(), true);
  sfc::assert_eq(some.is_none(), false);
  sfc::assert_eq(*some, 10);
}

SFC_TEST(ref) {
  auto val = 0;
  auto ref = Option<const int&>{val};
  auto mut_ref = Option<int&>{val};

  sfc::assert_eq(ref.is_some(), true);
  sfc::assert_eq(*ref, 0);

  sfc::assert_eq(mut_ref.is_some(), true);
  sfc::assert_eq(*mut_ref, 0);
}

SFC_TEST(copy_ctor) {
  const auto none = Option{};
  const auto some = Option{10};
  sfc::assert_eq(auto{none}, Option{});
  sfc::assert_eq(auto{some}, Option{10});
}

SFC_TEST(copy_assign) {
  const auto none = Option<int>{};
  const auto some = Option{10};

  auto x = Option<int>{};
  x = none;
  sfc::assert_eq(x.is_none(), true);

  x = some;
  sfc::assert_eq(x.is_some(), true);
  sfc::assert_eq(*x, 10);
}

SFC_TEST(move_ctor) {
  auto a = Option<String>{};
  auto b = Option{String::from("hello")};

  sfc::assert_eq(mem::move(a), Option{});
  sfc::assert_eq(mem::move(b), Option{Str{"hello"}});
}

SFC_TEST(move_assign) {
  auto a = Option<String>{};
  auto b = Option{String::from("hello")};

  auto x = Option<String>{};
  sfc::assert_eq(x.is_none(), true);

  x = mem::move(a);
  sfc::assert_eq(x, Option{});
  sfc::assert_eq(x.is_none(), true);

  x = mem::move(b);
  sfc::assert_eq(x, Option{"hello"});
  sfc::assert_eq(x.is_some(), true);
  sfc::assert_eq(*x, "hello");

  auto y = Option{String::from("world")};
  sfc::assert_eq(*y, "world");

  y = mem::move(x);
  sfc::assert_eq(y.is_some(), true);
  sfc::assert_eq(*y, "hello");
}

SFC_TEST(eq) {
  sfc::assert_eq(Option{}, Option{});
  sfc::assert_eq(Option{1}, Option{1});

  sfc::assert_ne(Option{}, Option{1});
  sfc::assert_ne(Option{1}, Option{2});
}

SFC_TEST(unwrap) {
  sfc::assert_eq(Option{10}.unwrap(), 10);
  sfc::assert_eq(Option{10}.unwrap_or(5), 10);
}

SFC_TEST(and_or) {
  sfc::assert_eq(Option{} & Option{1}, Option{});
  sfc::assert_eq(Option{10} & Option{1}, Option{1});

  sfc::assert_eq(Option{} | Option{1}, Option{1});
  sfc::assert_eq(Option{10} | Option{1}, Option{10});
}

SFC_TEST(unwrap_or_else) {
  auto make_opt = [] { return 1; };
  sfc::assert_eq(Option{10}.unwrap_or_else(make_opt), 10);
}

SFC_TEST(and_then) {
  auto add1 = [](auto x) { return Option{x + 1}; };

  sfc::assert_eq(Option{10}.and_then(add1), Option{11});
  sfc::assert_eq(Option{}.and_then(add1), Option{});
}

SFC_TEST(or_else) {
  auto make_opt = [] { return Option{1}; };

  sfc::assert_eq(Option{}.or_else(make_opt), Option{1});
  sfc::assert_eq(Option{10}.or_else(make_opt), Option{10});
}

SFC_TEST(map) {
  auto add1 = [](auto x) { return x + 1; };

  sfc::assert_eq(Option{}.map(add1), Option{});
  sfc::assert_eq(Option{10}.map(add1), Option{11});
}

SFC_TEST(map_or) {
  auto add1 = [](auto x) { return x + 1; };

  sfc::assert_eq(Option{}.map_or(5, add1), 5);
  sfc::assert_eq(Option{10}.map_or(5, add1), 11);
}

}  // namespace sfc::option::test
