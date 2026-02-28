#include "sfc/collections/queue.h"
#include "sfc/test/test.h"
#include "sfc/io.h"

namespace sfc::collections::queue::test {

SFC_TEST(push_pop) {
  auto q = Queue<int>{};
  for (auto i = 0U; i < 6U; ++i) {
    q.push(i);
  }
  sfc::expect_eq(q.len(), 6U);
  sfc::expect_eq(q.top(), Option{0});

  for (auto i = 0U; i < 2; ++i) {
    const auto x = q.pop();
    sfc::expect_eq(x, Option{i});
  }
  sfc::expect_eq(q.len(), 4U);
  sfc::expect_eq(q.top(), Option{2});

  for (auto i = 6U; i < 8U; ++i) {
    q.push(i);
  }
  sfc::expect_eq(q.len(), 6U);
  sfc::expect_eq(q.top(), Option{2});
}

SFC_TEST(reserve) {
  auto q = Queue<int>{};
  q.push(0);
  q.reserve(10);

  auto [sa, _] = q.as_slices();
  for (auto i = 1; i < 10; ++i) {
    q.push(i);
  }
  auto [sb, _] = q.as_slices();
  sfc::expect_eq(sa._ptr, sb._ptr);
}

SFC_TEST(iter) {
  auto q = Queue<int>{};
  for (auto i = 0; i < 10; ++i) {
    q.push(i);
  }

  auto iter = q.iter();
  for (auto i = 0; i < 10; ++i) {
    sfc::expect_eq(iter.next(), Option{i});
  }
  sfc::expect_false(iter);
}

}  // namespace sfc::collections::queue::test
