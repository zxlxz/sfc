#include "sfc/test.h"
#include "sfc/core/option.h"

namespace sfc::option::test {

SFC_TEST(simple) {
  {
    const auto opt = Option<int>{};
    panicking::assert_false(opt.is_some());
    panicking::assert_true(opt.is_none());
  }

  {
    const auto opt = Option<int>{10};
    panicking::assert_true(opt.is_some());
    panicking::assert_false(opt.is_none());

    panicking::assert_eq(*opt, 10);
  }
}

SFC_TEST(eq) {
  const auto o0 = Option<int>{};
  const auto o1 = Option<int>{1};
  const auto o2 = Option<int>{2};

  panicking::assert_eq(o0, o0);
  panicking::assert_eq(o1, o1);

  panicking::assert_ne(o0, o1);
  panicking::assert_ne(o1, o2);

  panicking::assert_eq(*o1, 1);
  panicking::assert_eq(*o2, 2);
}

SFC_TEST(unwrap) {
  panicking::assert_eq(Option<int>{}.unwrap_or(1), 1);

  panicking::assert_eq(Option<int>{10}.unwrap(), 10);
  panicking::assert_eq(Option<int>{10}.unwrap_or(5), 10);
}

SFC_TEST(and) {
  const auto a = Option<int>{};
  const auto b = Option<int>{10};

  panicking::assert_eq(a and Option{1}, Option<int>{});
  panicking::assert_eq(b and Option{1}, Option<int>{1});
}

SFC_TEST(or) {
  auto a = Option<int>{};
  auto b = Option<int>{10};

  panicking::assert_eq(mem::move(a) or Option{1}, Option{1});
  panicking::assert_eq(mem::move(b) or Option{1}, Option{10});
}

SFC_TEST(and_then) {
  auto a = Option<int>{10};
  auto b = Option<int>{};

  panicking::assert_eq(mem::move(a).and_then([](int x) { return Option{x + 1}; }), Option{11});
  panicking::assert_eq(mem::move(b).and_then([](int x) { return Option{x + 1}; }), Option<int>{});
}

SFC_TEST(or_else) {
  auto a = Option<int>{};
  auto b = Option<int>{10};

  panicking::assert_eq(mem::move(a).or_else([] { return Option{1}; }), Option{1});
  panicking::assert_eq(mem::move(b).or_else([] { return Option{1}; }), Option{10});
}

SFC_TEST(map) {
  auto a = Option<int>{};
  auto b = Option<int>{10};

  panicking::assert_eq(mem::move(a).map([](int x) { return x + 1; }), Option<int>{});
  panicking::assert_eq(mem::move(b).map([](int x) { return x + 1; }), Option{11});
}

SFC_TEST(map_or) {
  auto a = Option<int>{};
  auto b = Option<int>{10};

  panicking::assert_eq(mem::move(a).map_or(5, [](int x) { return x + 1; }), 5);
  panicking::assert_eq(mem::move(b).map_or(5, [](int x) { return x + 1; }), 11);
}

}  // namespace sfc::option::test
