#include "sfc/core.h"
#include "sfc/test/test.h"

namespace sfc::iter::test {

SFC_TEST(advance_by) {
  const int v[] = {0, 1, 2, 3};

  auto iter = Slice{v}.iter();

  sfc::assert_eq(iter.advance_by(2), 2U);
  sfc::assert_eq(iter.next(), Option{2});

  sfc::assert_eq(iter.advance_by(4), 1U);
  sfc::assert_eq(iter.next(), None{});
}

SFC_TEST(find) {
  const int v[] = {0, 1, 2, 3};

  auto iter = Slice{v}.iter();
  sfc::assert_eq(iter.find([](auto& x) { return x == 1; }), Option{1});
  sfc::assert_eq(iter.next(), Option{2});
}

SFC_TEST(rfind) {
  const int v[] = {0, 1, 2, 3};

  auto iter = Slice{v}.iter();
  sfc::assert_eq(iter.rfind([](auto& x) { return x == 3; }), Option{3});
  sfc::assert_eq(iter.next_back(), Option{2});
}

SFC_TEST(position) {
  const int v[] = {0, 1, 2, 3};

  auto iter = Slice{v}.iter();
  sfc::assert_eq(iter.position([](auto& x) { return x == 2; }), Option{2U});
  sfc::assert_eq(iter.next(), Option{3});
}

SFC_TEST(rposition) {
  const int v[] = {0, 1, 2, 3};

  auto iter = Slice{v}.iter();
  sfc::assert_eq(iter.rposition([](auto& x) { return x == 2; }), Option{2U});
  sfc::assert_eq(iter.next_back(), Option{1});
}

SFC_TEST(all) {
  const int v[] = {0, 1, 2, 3};

  auto iter = Slice{v}.iter();
  sfc::assert_eq(iter.all([](auto x) { return x != 2; }), false);
  sfc::assert_eq(iter.next(), Option{3});
}

SFC_TEST(any) {
  const int v[] = {0, 1, 2, 3};

  auto iter = Slice{v}.iter();
  sfc::assert_eq(iter.any([](auto x) { return x == 2; }), true);
  sfc::assert_eq(iter.next(), Option{3});
}

SFC_TEST(try_for_each) {
  const int v[] = {0, 1, 2, 3};

  auto iter = Slice{v}.iter();

  auto sum = 0;
  iter.try_for_each([&](auto x) {
    sum += x;
    return x != 2;
  });
  sfc::assert_eq(sum, 3);
  sfc::assert_eq(iter.next(), Option{3});
}

SFC_TEST(for_each) {
  const int v[] = {0, 1, 2, 3};

  auto sum = 0;
  Slice{v}.iter().for_each([&](auto x) { sum += x; });
  sfc::assert_eq(sum, 6);
}

SFC_TEST(fold) {
  const int v[] = {0, 1, 2, 3};

  auto sum = Slice{v}.iter().fold(0, [](auto acc, auto x) { return acc + x; });
  sfc::assert_eq(sum, 6);
}

SFC_TEST(reduce) {
  const int v[] = {0, 1, 2, 3};

  auto min_ref = Slice{v}.iter().reduce([](auto& a, auto& b) -> auto& { return a < b ? a : b; });
  sfc::assert_eq(min_ref, Option{v[0]});

  auto max_ref = Slice{v}.iter().reduce([](auto& a, auto& b) -> auto& { return a > b ? a : b; });
  sfc::assert_eq(max_ref, Option{v[3]});
}

SFC_TEST(count) {
  const int v[] = {0, 1, 2, 3};

  auto iter = Slice{v}.iter();
  sfc::assert_eq(iter.count(), 4U);
}

SFC_TEST(min) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto min_val = Slice{v}.iter().min();
  sfc::assert_eq(min_val, Option{0});
}

SFC_TEST(max) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto max_val = Slice{v}.iter().max();
  sfc::assert_eq(max_val, Option{5});
}

SFC_TEST(min_by_key) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto min_val = Slice{v}.iter().min_by_key([](auto& x) { return -x; });
  sfc::assert_eq(min_val, Option{5});
}

SFC_TEST(max_by_key) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto max_val = Slice{v}.iter().max_by_key([](auto& x) { return -x; });
  sfc::assert_eq(max_val, Option{0});
}

SFC_TEST(collect) {
  const int v[] = {0, 1, 2, 3};

  auto list = Slice{v}.iter().collect(List<int>{});
  for (auto x : v) {
    sfc::assert_eq(x, list[u32(x)]);
  }
}

SFC_TEST(collect_into) {
  const int v[] = {0, 1, 2, 3};

  List<int> list;
  Slice{v}.iter().collect_into(list);
  for (auto x : v) {
    sfc::assert_eq(x, list[u32(x)]);
  }
}

SFC_TEST(take) {
  const int v[] = {0, 1, 2, 3};

  auto iter = Slice{v}.iter().take(2);
  sfc::assert_eq(iter.next(), Option{0});
  sfc::assert_eq(iter.next(), Option{1});
  sfc::assert_eq(iter.next(), None{});
}

}  // namespace sfc::iter::test
