#include "sfc/collections/queue.h"
#include "sfc/test/ut.h"

namespace sfc::collections::test {

SFC_TEST(queue_push_pop) {
  auto q = Queue<int>{};
  q.push(1);

  panicking::expect_eq(q.len(), 1U);
  panicking::expect_eq(q.top(), Option{1});

  q.push(2);
  panicking::expect_eq(q.len(), 2U);
  panicking::expect_eq(q.top(), Option{1});
  panicking::expect_eq(q.pop(), Option{1});
  
  panicking::expect_eq(q.top(), Option{2});
  panicking::expect_eq(q.pop(), Option{2});
  panicking::expect(!q.pop());
}

}  // namespace sfc::collections::test
