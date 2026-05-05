#include "sfc/test/test.h"
#include "sfc/core/result.h"

namespace sfc::result::test {

enum E {
  E0,
  EA,
  EB,
};

SFC_TEST(simple) {
  using Res = result::Result<int, E>;

  {
    const auto res = Res{1};
    sfc::expect_true(res.is_ok());
    sfc::expect_false(res.is_err());
  }

  {
    const auto res = Res{EA};
    sfc::expect_false(res.is_ok());
    sfc::expect_true(res.is_err());
  }
}

SFC_TEST(to_option) {
  using Res = result::Result<int, E>;

  sfc::expect_eq(Res{1}.ok(), Option{1});
  sfc::expect_eq(Res{1}.err(), Option<E>{});

  const auto res = Res{EA};
  sfc::expect_eq(Res{EA}.ok(), Option<int>{});
  sfc::expect_eq(Res{EA}.err(), Option{EA});
}

SFC_TEST(unwrap) {
  using Res = result::Result<int, E>;

  sfc::expect_eq(Res{1}.unwrap(), 1);
  sfc::expect_eq(Res{1}.unwrap_or(2), 1);

  sfc::expect_eq(Res{EA}.unwrap_or(2), 2);
  sfc::expect_eq(Res{EA}.unwrap_err(), EA);
}

SFC_TEST(and_or) {
  using Res = result::Result<int, E>;

  sfc::expect_eq((Res{1} & Res{2}), Res{2});
  sfc::expect_eq((Res{EA} & Res{2}), Res{EA});

  sfc::expect_eq((Res{1} | Res{2}), Res{1});
  sfc::expect_eq((Res{EA} | Res{2}), Res{2});
}

SFC_TEST(and_then) {
  using Res = result::Result<int, E>;

  auto add1 = [](auto x) { return Res{x + 1}; };

  sfc::expect_eq(Res{10}.and_then(add1), Res{11});
  sfc::expect_eq(Res{EA}.and_then(add1), Res{EA});
}

SFC_TEST(or_else) {
  using Res = result::Result<int, E>;
  auto ret1 = [] { return Res{1}; };

  sfc::expect_eq(Res{10}.or_else(ret1), Res{10});
  sfc::expect_eq(Res{EA}.or_else(ret1), Res{1});
}

SFC_TEST(map) {
  using Res = result::Result<int, E>;
  auto add1 = [](auto x) { return x + 1; };

  sfc::expect_eq(Res{10}.map(add1), Res{11});
  sfc::expect_eq(Res{EA}.map(add1), Res{EA});
}

SFC_TEST(map_err) {
  using Res = result::Result<int, E>;
  auto toEB = [](auto) { return EB; };

  sfc::expect_eq(Res{10}.map_err(toEB), Res{10});
  sfc::expect_eq(Res{EA}.map_err(toEB), Res{EB});
}

}  // namespace sfc::result::test
