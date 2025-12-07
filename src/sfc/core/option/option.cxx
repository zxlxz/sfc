#include "sfc/test/ut.h"
#include "sfc/core/option.h"

namespace sfc::option::test {

struct T {
  int _val = 0;

 public:
  T() : _val{1} {
    count() += _val;
  }

  T(T&& other) : _val{other._val} {
    other._val = 0;
  }

  ~T() {
    count() -= _val;
  }

  static auto count() -> i32& {
    static i32 cnt = 0;
    return cnt;
  }
};

SFC_TEST(basic) {
  using Option = option::Option<int>;

  const auto none = Option{};
  panicking::expect(none.is_none());
  panicking::expect(!none.is_some());

  const auto some = Option{10};
  panicking::expect(some.is_some());
  panicking::expect(!some.is_none());
  panicking::expect_eq(*some, 10);
}

SFC_TEST(copy) {
  using Option = option::Option<int>;
  const auto none = Option{};
  const auto some = Option{10};

  // copy-ctor
  {
    const auto a = none;
    const auto b = some;
    panicking::expect(a.is_none());
    panicking::expect(b.is_some());
    panicking::expect_eq(*b, 10);
  }

  // assign
  {
    auto x = Option{};
    x = none;
    panicking::expect(x.is_none());

    x = some;
    panicking::expect(x.is_some());
    panicking::expect_eq(*x, 10);
  }
}

SFC_TEST(move) {
  using Option = option::Option<T>;

  auto a = Option{};
  auto b = Option{T{}};
  panicking::expect_eq(T::count(), 1);

  // move
  {
    auto x = mem::move(a);
    panicking::expect_eq(T::count(), 1);

    auto y = mem::move(b);
    panicking::expect_eq(T::count(), 1);
  }
  panicking::expect_eq(T::count(), 0);

  // assign
  {
    auto x = Option{};
    auto y = Option{T{}};
    panicking::expect_eq(T::count(), 1);

    x = mem::move(a);
    panicking::expect_eq(T::count(), 1);

    x = mem::move(b);
    panicking::expect_eq(T::count(), 1);

    y = mem::move(a);
    panicking::expect_eq(T::count(), 0);

    y = mem::move(b);
    panicking::expect_eq(T::count(), 0);
  }
  panicking::expect_eq(T::count(), 0);
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
