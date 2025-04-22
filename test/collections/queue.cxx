#include "sfc/collections.h"
#include "sfc/test.h"

namespace sfc::collections::queue {

SFC_TEST(push) {
  auto q = Queue<int>{};
  q.push(1);

  test::assert_eq(q.len(), 1U);
  test::assert_eq(q.first(), 1);
  test::assert_eq(q.last(), 1);

  q.push(2);
  test::assert_eq(q.len(), 2U);
  test::assert_eq(q.first(), 1);
  test::assert_eq(q.last(), 2);
}

SFC_TEST(pop) {
  auto q = Queue<int>{};
  q.push(1);
  q.push(2);
  q.push(3);

  test::assert_eq(q.pop(), Option{1});
  test::assert_eq(q.pop(), Option{2});
  test::assert_eq(q.pop(), Option{3});
  test::assert_false(q.pop());
}

SFC_TEST(insert) {
  auto q = Queue<int>{};
  q.push(1);
  q.push(3);

  q.insert(1, 2);
  test::assert_eq(q[1], 2);
  q.insert(0, 0);
  test::assert_eq(q[0], 0);

  q.insert(8, 4);
  test::assert_eq(q.len(), 5U);
  test::assert_eq(q.last(), 4);
}

}  // namespace sfc::collections::queue
