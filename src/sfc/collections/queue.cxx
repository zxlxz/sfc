#include "sfc/collections/queue.h"
#include "sfc/test/test.h"
#include "sfc/io.h"

namespace sfc::collections::queue::test {

SFC_TEST(push_pop) {
  auto q = Queue<int>{};
  for (auto i = 0; i < 6; ++i) {
    q.push(i);
  }
  sfc::assert_eq(q.len(), 6U);
  sfc::assert_eq(q.top(), Option{0});

  for (auto i = 0; i < 2; ++i) {
    const auto x = q.pop();
    sfc::assert_eq(x, Option{i});
  }
  sfc::assert_eq(q.len(), 4U);
  sfc::assert_eq(q.top(), Option{2});

  for (auto i = 6; i < 8; ++i) {
    q.push(i);
  }
  sfc::assert_eq(q.len(), 6U);
  sfc::assert_eq(q.top(), Option{2});
}

SFC_TEST(reserve) {
  auto q = Queue<int>{};
  q.push(0);
  q.reserve(10);

  auto sa = q.as_slices();
  for (auto i = 1; i < 10; ++i) {
    q.push(i);
  }
  auto sb = q.as_slices();
  sfc::assert_eq(sa._0._ptr, sb._0._ptr);
}

SFC_TEST(iter) {
  auto q = Queue<int>::with_capacity(10);
  for (auto i = 0; i < 10; ++i) {
    q.push(i);
  }

  auto iter = q.iter();
  for (auto i = 0; i < 10; ++i) {
    sfc::assert_eq(iter.next(), Option{i});
  }
  sfc::assert_eq(iter.next(), None{});
}

}  // namespace sfc::collections::queue::test
