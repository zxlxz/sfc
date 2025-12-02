#include "sfc/test.h"

namespace sfc::slice::test {

SFC_TEST(simple) {
  {
    const auto a = Slice<int>{};
    panicking::expect_eq(a.len(), 0U);
    panicking::expect(a.is_empty());
    panicking::expect(!a);
  }

  {
    const int v[] = {1, 2, 3};
    const auto b = Slice{v};
    panicking::expect_eq(b.as_ptr(), b._ptr);
    panicking::expect_eq(b.len(), 3U);
    panicking::expect(b);
    panicking::expect(!b.is_empty());
  }
}

SFC_TEST(index) {
  const int v[] = {1, 2, 3};
  const auto s = Slice{v};
  panicking::expect_eq(s.len(), 3U);
  panicking::expect(s);
  panicking::expect(s.is_empty() == false);
  panicking::expect_eq(s[0], 1);
  panicking::expect_eq(s[1], 2);
  panicking::expect_eq(s[2], 3);
}

SFC_TEST(slice) {
  const int v[] = {1, 2, 3};
  const auto a = Slice{v};

  {
    const auto b = a[{1, 3}];
    panicking::expect_eq(b.len(), 2U);
    panicking::expect_eq(b[0], 2);
    panicking::expect_eq(b[1], 3);
  }

  {
    const auto c = a[{0, 3}];
    panicking::expect_eq(c.len(), 3U);
    panicking::expect_eq(c[0], 1);
    panicking::expect_eq(c[1], 2);
    panicking::expect_eq(c[2], 3);
  }

  {
    const auto d = a[{2, $}];
    panicking::expect_eq(d.len(), 1U);
    panicking::expect_eq(d[0], 3);
  }

  {
    const auto e = a[{5, 6}];
    panicking::expect_eq(e.len(), 0U);
    panicking::expect(e.is_empty());
    panicking::expect(!e);
  }
}

SFC_TEST(split_at) {
  const int v[] = {1, 2, 3};
  const auto a = Slice{v};

  {
    const auto [b, c] = a.split_at(1);
    panicking::expect_eq(b.len(), 1U);
    panicking::expect_eq(b[0], 1);
    panicking::expect_eq(c.len(), 2U);
    panicking::expect_eq(c[0], 2);
    panicking::expect_eq(c[1], 3);
  }

  {
    const auto [d, e] = a.split_at(3);
    panicking::expect_eq(d.len(), 3U);
    panicking::expect_eq(d[0], 1);
    panicking::expect_eq(d[1], 2);
    panicking::expect_eq(d[2], 3);
    panicking::expect_eq(e.len(), 0U);
    panicking::expect(e.is_empty());
    panicking::expect(!e);
  }
}

SFC_TEST(swap) {
  int v[] = {1, 2, 3};
  auto a = Slice{v};
  a.swap(0, 2);
  panicking::expect_eq(a[0], 3);
  panicking::expect_eq(a[1], 2);
  panicking::expect_eq(a[2], 1);
}

SFC_TEST(fill) {
  int v[] = {1, 2, 3};
  auto a = Slice{v};
  a.fill(0);
  panicking::expect_eq(a[0], 0);
  panicking::expect_eq(a[1], 0);
  panicking::expect_eq(a[2], 0);
}

SFC_TEST(eq) {
  const int v1[] = {1, 2, 3};
  const int v2[] = {1, 2, 3};
  const int v3[] = {4, 5, 6};
  const auto a = Slice{v1};
  const auto b = Slice{v2};
  const auto c = Slice{v3};

  panicking::expect_eq(a, b);
  panicking::expect_ne(a, c);
}

SFC_TEST(contains) {
  const int v[] = {1, 2, 3};
  const auto s = Slice{v};
  panicking::expect(s.contains(2));
  panicking::expect(!s.contains(4));
}

SFC_TEST(starts_with) {
  const int v[] = {1, 2, 3};
  const auto s = Slice{v};

  panicking::expect(s.starts_with({v + 0, 2}));
  panicking::expect(!s.starts_with({v + 1, 2}));
}

SFC_TEST(ends_with) {
  const int v[] = {1, 2, 3};
  const auto s = Slice{v};
  panicking::expect(s.ends_with({v + 1, 2}));
  panicking::expect(!s.ends_with({v + 0, 2}));
}

SFC_TEST(iter) {
  const int v[] = {1, 2, 3};
  const auto s = Slice{v};

  auto it = s.iter();
  panicking::expect_eq(it.next(), Option{1});
  panicking::expect_eq(it.next(), Option{2});
  panicking::expect_eq(it.next(), Option{3});
  panicking::expect(!it.next());
}

SFC_TEST(windows) {
  const int v[] = {1, 2, 3};
  const auto s = Slice{v};

  {
    auto w = s.windows(2);
    panicking::expect_eq(w.next(), Option{Slice{v + 0, 2}});
    panicking::expect_eq(w.next(), Option{Slice{v + 1, 2}});
    panicking::expect(!w.next());
  }

  {
    auto w = s.windows(3);
    panicking::expect_eq(w.next(), Option{Slice{v + 0, 3}});
    panicking::expect(!w.next());
  }
}

SFC_TEST(chunks) {
  const int v[] = {1, 2, 3};
  const auto s = Slice{v};

  {
    auto c = s.chunks(2);
    panicking::expect_eq(c.next(), Option{Slice{v + 0, 2}});
    panicking::expect_eq(c.next(), Option{Slice{v + 2, 1}});
    panicking::expect(!c.next());
  }

  {
    auto c = s.chunks(3);
    panicking::expect_eq(c.next(), Option{Slice{v + 0, 3}});
    panicking::expect(!c.next());
  }
}

}  // namespace sfc::slice
