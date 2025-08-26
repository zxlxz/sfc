#include "sfc/core.h"
#include "sfc/test.h"

namespace sfc::iter::test {

SFC_TEST(fold) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto sum = Slice{v}.iter()->fold(0, [](auto acc, auto x) { return acc + x; });
  panicking::assert_eq(sum, 15);
}

SFC_TEST(reduce) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto min_val = Slice{v}.iter()->reduce([](auto a, auto b) { return a < b ? a : b; });
  panicking::assert_true(min_val);
  panicking::assert_eq(*min_val, 0);

  auto max_val = Slice{v}.iter()->reduce([](auto a, auto b) { return a > b ? a : b; });
  panicking::assert_true(max_val);
  panicking::assert_eq(*max_val, 5);

  auto min_ref = Slice{v}.iter()->reduce([](auto& a, auto& b) -> auto& { return a < b ? a : b; });
  panicking::assert_true(min_ref);
  panicking::assert_eq(&*min_ref, &v[0]);

  auto max_ref = Slice{v}.iter()->reduce([](auto& a, auto& b) -> auto& { return a > b ? a : b; });
  panicking::assert_true(max_ref);
  panicking::assert_eq(&*max_ref, &v[5]);
}

SFC_TEST(find) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto pos = Slice{v}.iter()->find([](auto& x) { return x == 3; });
  panicking::assert_true(pos);
  panicking::assert_eq(*pos, 3);
}

SFC_TEST(rfind) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto pos = Slice{v}.iter()->rfind([](auto& x) { return x == 3; });
  panicking::assert_true(pos);
  panicking::assert_eq(*pos, 3);
}

SFC_TEST(position) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto pos = Slice{v}.iter()->position([](auto& x) { return x == 3; });
  panicking::assert_true(pos);
  panicking::assert_eq(*pos, 3);
}

SFC_TEST(rposition) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto pos = Slice{v}.iter()->rposition([](auto& x) { return x == 3; });
  panicking::assert_true(pos);
  panicking::assert_eq(*pos, 3);
}

SFC_TEST(all) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto all_even = Slice{v}.iter()->all([](auto& x) { return x % 2 == 0; });
  panicking::assert_false(all_even);

  auto all_non_negative = Slice{v}.iter()->all([](auto& x) { return x >= 0; });
  panicking::assert_true(all_non_negative);
}

SFC_TEST(any) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto any_even = Slice{v}.iter()->any([](auto& x) { return x % 2 == 0; });
  panicking::assert_true(any_even);

  auto any_greater_than_five = Slice{v}.iter()->any([](auto& x) { return x > 5; });
  panicking::assert_false(any_greater_than_five);
}

SFC_TEST(min) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto min_val = Slice{v}.iter()->min();
  panicking::assert_true(min_val);
  panicking::assert_eq(*min_val, 0);
}

SFC_TEST(max) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto max_val = Slice{v}.iter()->max();
  panicking::assert_true(max_val);
  panicking::assert_eq(*max_val, 5);
}

SFC_TEST(min_by_key) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto min_val = Slice{v}.iter()->min_by_key([](auto& x) { return -x; });
  panicking::assert_true(min_val);
  panicking::assert_eq(*min_val, 5);
}

SFC_TEST(max_by_key) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto max_val = Slice{v}.iter()->max_by_key([](auto& x) { return -x; });
  panicking::assert_true(max_val);
  panicking::assert_eq(*max_val, 0);
}

SFC_TEST(sum) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto sum = Slice{v}.iter()->sum();
  panicking::assert_eq(sum, 15);
}

SFC_TEST(product) {
  const int v[] = {1, 2, 3, 4, 5};

  auto prod = Slice{v}.iter()->product();
  panicking::assert_eq(prod, 120);
}

SFC_TEST(collect) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto collected = Slice{v}.iter()->collect<Vec<int>>();
  panicking::assert_eq(collected.len(), 6);
  for (auto i : v) {
    panicking::assert_eq(collected[i], i);
  }
}

SFC_TEST(rev) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto rev = Slice{v}.iter()->rev();
  for (auto i = 0; i < 6; ++i) {
    auto opt = rev.next();
    panicking::assert_true(opt);
    panicking::assert_eq(*opt, 5 - i);
  }
}

SFC_TEST(map) {
  const int v[] = {0, 1, 2, 3, 4, 5};

  auto mapped = Slice{v}.iter()->map([](auto& x) { return x * 2; });
  for (auto i = 0; i < 6; ++i) {
    auto opt = mapped.next();
    panicking::assert_true(opt);
    panicking::assert_eq(*opt, v[i] * 2);
  }
}

SFC_TEST(zip) {
  const int a[] = {0, 1, 2, 3, 4, 5};
  const char b[] = {'a', 'b', 'c', 'd', 'e', 'f'};

  auto zipped = zip(Slice{a}.iter(), Slice{b}.iter());
  for (auto i = 0; i < 6; ++i) {
    auto opt = zipped.next();
    panicking::assert_true(opt);

    const auto [x, y] = *opt;
    panicking::assert_eq(x, a[i]);
    panicking::assert_eq(y, b[i]);
  }

  panicking::assert_false(zipped.next());
}

}  // namespace sfc::iter::test
