#include "sfc/test/test.h"

namespace sfc::slice::test {

SFC_TEST(simple) {
  {
    const auto a = Slice<int>{};
    sfc::assert_eq(a.len(), 0U);
    sfc::assert_eq(a.is_empty(), true);
  }

  {
    const int v[] = {1, 2, 3};
    const auto b = Slice{v};
    sfc::assert_eq(b.as_ptr(), b._ptr);
    sfc::assert_eq(b.len(), 3U);
    sfc::assert_eq(b.is_empty(), false);
  }
}

SFC_TEST(index) {
  const int v[] = {1, 2, 3};
  const auto s = Slice{v};
  sfc::assert_eq(s.len(), 3U);
  sfc::assert_eq(s.is_empty(), false);
  sfc::assert_eq(s[0], 1);
  sfc::assert_eq(s[1], 2);
  sfc::assert_eq(s[2], 3);
}

SFC_TEST(slice) {
  const int v[] = {1, 2, 3};
  const auto a = Slice{v};

  {
    const auto b = a[{1, 3}];
    sfc::assert_eq(b.len(), 2U);
    sfc::assert_eq(b[0], 2);
    sfc::assert_eq(b[1], 3);
  }

  {
    const auto c = a[{0, 3}];
    sfc::assert_eq(c.len(), 3U);
    sfc::assert_eq(c[0], 1);
    sfc::assert_eq(c[1], 2);
    sfc::assert_eq(c[2], 3);
  }

  {
    const auto d = a[{2, $}];
    sfc::assert_eq(d.len(), 1U);
    sfc::assert_eq(d[0], 3);
  }

  {
    const auto e = a[{5, 6}];
    sfc::assert_eq(e.len(), 0U);
    sfc::assert_eq(e.is_empty(), true);
  }
}

SFC_TEST(split_at) {
  const int v[] = {1, 2, 3};
  const auto a = Slice{v};

  const auto [b, c] = a.split_at(1);
  sfc::assert_eq(b.len(), 1U);
  sfc::assert_eq(b[0], 1);
  sfc::assert_eq(c.len(), 2U);
  sfc::assert_eq(c[0], 2);
  sfc::assert_eq(c[1], 3);

  const auto [d, e] = a.split_at(3);
  sfc::assert_eq(d.len(), 3U);
  sfc::assert_eq(d[0], 1);
  sfc::assert_eq(d[1], 2);
  sfc::assert_eq(d[2], 3);
  sfc::assert_eq(e.len(), 0U);
  sfc::assert_eq(e.is_empty(), true);
}

SFC_TEST(swap) {
  int v[] = {1, 2, 3};

  auto a = Slice{v};
  a.swap(0, 2);
  sfc::assert_eq(a[0], 3);
  sfc::assert_eq(a[1], 2);
  sfc::assert_eq(a[2], 1);
}

SFC_TEST(fill) {
  int v[] = {1, 2, 3};
  auto a = Slice{v};
  a.fill(0);
  sfc::assert_eq(a[0], 0);
  sfc::assert_eq(a[1], 0);
  sfc::assert_eq(a[2], 0);
}

SFC_TEST(eq) {
  const int v1[] = {1, 2, 3};
  const int v2[] = {1, 2, 3};
  const int v3[] = {4, 5, 6};
  const auto a = Slice{v1};
  const auto b = Slice{v2};
  const auto c = Slice{v3};

  sfc::assert_eq(a, b);
  sfc::assert_ne(a, c);
}

SFC_TEST(contains) {
  const int v[] = {1, 2, 3};
  const auto s = Slice{v};
  sfc::assert_eq(s.contains(2), true);
  sfc::assert_eq(s.contains(4), false);
}

SFC_TEST(find) {
  const int v[] = {1, 2, 3, 2};
  const auto s = Slice{v};
  sfc::assert_eq(s.find(2), Option{1UZ});
  sfc::assert_eq(s.find(4), None{});
}

SFC_TEST(rfind) {
  const int v[] = {1, 2, 3, 2};
  const auto s = Slice{v};
  sfc::assert_eq(s.rfind(2), Option{3UZ});
  sfc::assert_eq(s.rfind(4), None{});
}

SFC_TEST(starts_with) {
  const int v[] = {1, 2, 3};
  const auto s = Slice{v};

  sfc::assert_eq(s.starts_with({v + 0, 2}), true);
  sfc::assert_eq(s.starts_with({v + 1, 2}), false);
}

SFC_TEST(ends_with) {
  const int v[] = {1, 2, 3};
  const auto s = Slice{v};
  sfc::assert_eq(s.ends_with({v + 1, 2}), true);
  sfc::assert_eq(s.ends_with({v + 0, 2}), false);
}

SFC_TEST(iter) {
  const int v[] = {1, 2, 3};
  const auto s = Slice{v};

  auto it = s.iter();
  sfc::assert_eq(it.next(), Option{1});
  sfc::assert_eq(it.next(), Option{2});
  sfc::assert_eq(it.next(), Option{3});
  sfc::assert_eq(it.next().is_some(), false);
}

SFC_TEST(windows) {
  const int v[] = {1, 2, 3};
  const auto s = Slice{v};

  // forward: 2
  {
    auto w = s.windows(2);
    sfc::assert_eq(w.next(), Option{Slice{v + 0, 2}});
    sfc::assert_eq(w.next(), Option{Slice{v + 1, 2}});
    sfc::assert_eq(w.next().is_some(), false);
  }

  // forward: 3
  {
    auto w = s.windows(3);
    sfc::assert_eq(w.next(), Option{Slice{v + 0, 3}});
    sfc::assert_eq(w.next().is_some(), false);
  }

  // backward: 2
  {
    auto w = s.windows(2);
    sfc::assert_eq(w.next_back(), Option{Slice{v + 1, 2}});
    sfc::assert_eq(w.next_back(), Option{Slice{v + 0, 2}});
    sfc::assert_eq(w.next_back().is_some(), false);
  }
}

SFC_TEST(chunks) {
  const int v[] = {1, 2, 3};
  const auto s = Slice{v};

  {
    auto c = s.chunks(2);
    sfc::assert_eq(c.next(), Option{Slice{v + 0, 2}});
    sfc::assert_eq(c.next(), Option{Slice{v + 2, 1}});
    sfc::assert_eq(c.next().is_some(), false);
  }

  {
    auto c = s.chunks(3);
    sfc::assert_eq(c.next(), Option{Slice{v + 0, 3}});
    sfc::assert_eq(c.next().is_some(), false);
  }
}

}  // namespace sfc::slice::test
