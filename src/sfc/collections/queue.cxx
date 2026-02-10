#include "sfc/collections/queue.h"
#include "sfc/test/test.h"

namespace sfc::collections::queue::test {

SFC_TEST(push_pop) {
  auto q = Queue<int>{};
  q.push(1);

  sfc::expect_eq(q.len(), 1U);
  sfc::expect_eq(q.top(), Option{1});

  q.push(2);
  sfc::expect_eq(q.len(), 2U);
  sfc::expect_eq(q.top(), Option{1});
  sfc::expect_eq(q.pop(), Option{1});

  sfc::expect_eq(q.top(), Option{2});
  sfc::expect_eq(q.pop(), Option{2});
  sfc::expect_false(q.pop());
}

}  // namespace sfc::collections::queue::test
