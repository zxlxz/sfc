#include "sfc/test.h"
#include "sfc/core/result.h"

namespace sfc::result::test {

enum class Error : i8 {
  Success = 0,
  EA,
  EB,
};

SFC_TEST(simple) {
  using Result = result::Result<int, Error>;

  const auto ok = Result{1};

  panicking::expect(ok.is_ok());
  panicking::expect(!ok.is_err());
  panicking::expect_eq(*ok, 1);

  const auto err = Result{Error::EA};
  panicking::expect(!err.is_ok());
  panicking::expect(err.is_err());
  panicking::expect_eq(~err, Error::EA);
}

SFC_TEST(unwrap) {
  using Result = result::Result<int, Error>;

  panicking::expect_eq(Result{1}.unwrap(), 1);
  panicking::expect_eq(Result{1}.unwrap_or(2), 1);

  panicking::expect_eq(Result{Error::EA}.unwrap_or(2), 2);
  panicking::expect_eq(Result{Error::EA}.unwrap_err(), Error::EA);
}

SFC_TEST(and_or) {
  using Result = result::Result<int, Error>;

  panicking::expect_eq((Result{1} & Result{2}), Result{2});
  panicking::expect_eq((Result{Error::EA} & Result{2}), Result{Error::EA});

  panicking::expect_eq((Result{1} | Result{2}), Result{1});
  panicking::expect_eq((Result{Error::EA} | Result{2}), Result{2});
}

SFC_TEST(and_then) {
  using Result = result::Result<int, Error>;

  auto add1 = [](auto x) { return Result{x + 1}; };

  panicking::expect_eq(Result{10}.and_then(add1), Result{11});
  panicking::expect_eq(Result{Error::EA}.and_then(add1), Result{Error::EA});
}

SFC_TEST(or_else) {
  using Result = result::Result<int, Error>;
  auto ret1 = [] { return Result{1}; };

  panicking::expect_eq(Result{10}.or_else(ret1), Result{10});
  panicking::expect_eq(Result{Error::EA}.or_else(ret1), Result{1});
}

SFC_TEST(map) {
  using Result = result::Result<int, Error>;
  auto add1 = [](auto x) { return x + 1; };

  panicking::expect_eq(Result{10}.map(add1), Result{11});
  panicking::expect_eq(Result{Error::EA}.map(add1), Result{Error::EA});
}

SFC_TEST(map_err) {
  using Result = result::Result<int, Error>;
  auto toEB = [](auto) { return Error::EB; };

  panicking::expect_eq(Result{10}.map_err(toEB), Result{10});
  panicking::expect_eq(Result{Error::EA}.map_err(toEB), Result{Error::EB});
}

}  // namespace sfc::result::test
