#include "sfc/test.h"

namespace sfc::future::test {

SFC_TEST(await) {
  auto val = 0;

  auto f = [](auto& val) -> Future<void> {
    for (int i = 0; i < 5; ++i) {
      val = i;
      co_await Suspend{};
    }
    co_return;
  }(val);

  for (auto i = 0; i < 5; ++i) {
    f.poll();
    panicking::assert_eq(i, val);
  }

  f.poll();
  panicking::assert_eq(4, val);
}

SFC_TEST(yield) {
  auto f = []() -> Future<int> {
    for (int i = 0; i < 5; ++i) {
      co_yield i;
    }
    co_return 5;
  }();

  for (auto i = 0; i < 5; ++i) {
    auto p = f.poll();
    panicking::assert_eq(i, *p);
  }
  auto p = f.poll();
  panicking::assert_eq(5, *p);
}

SFC_TEST(await_yield) {
  auto f1 = [&]() -> Future<int> {
    for (int i = 0; i < 5; ++i) {
      co_yield i;
    }
    co_return 5;
  }();

  auto f2 = [&]() -> Future<int> {
    auto sum = 0;
    for (int i = 0; i < 6; ++i) {
      const auto val = co_await f1;
      sum += val;
    }
    co_return sum;
  }();

  while (true) {
    auto p = f2.poll();
    if (p.is_ready()) {
      panicking::assert_eq(*p, 15);
      break;
    }
  }
}

}  // namespace sfc::future::test
