#include "sfc/collections.h"
#include "sfc/test/ut.h"

namespace sfc::collections::test {

SFC_TEST(queue_push_front) {
  auto q = VecDeque<int>{};
  q.push_front(1);

  panicking::expect_eq(q.len(), 1U);
  panicking::expect_eq(q.front(), 1);
  panicking::expect_eq(q.back(), 1);

  q.push_front(2);
  panicking::expect_eq(q.len(), 2U);
  panicking::expect_eq(q.front(), 2);
  panicking::expect_eq(q.back(), 1);
}

SFC_TEST(queue_push_back) {
  auto q = VecDeque<int>{};
  q.push_back(1);

  panicking::expect_eq(q.len(), 1U);
  panicking::expect_eq(q.front(), 1);
  panicking::expect_eq(q.back(), 1);

  q.push_back(2);
  panicking::expect_eq(q.len(), 2U);
  panicking::expect_eq(q.front(), 1);
  panicking::expect_eq(q.back(), 2);
}

SFC_TEST(queue_pop_front) {
  auto q = VecDeque<int>{};
  q.push_front(1);
  q.push_front(2);
  q.push_front(3);

  panicking::expect_eq(q.pop_front(), Option{3});
  panicking::expect_eq(q.pop_front(), Option{2});
  panicking::expect_eq(q.pop_front(), Option{1});
  panicking::expect(!q.pop_front());
}

SFC_TEST(queue_pop_back) {
  auto q = VecDeque<int>{};
  q.push_back(1);
  q.push_back(2);
  q.push_back(3);

  panicking::expect_eq(q.pop_back(), Option{3});
  panicking::expect_eq(q.pop_back(), Option{2});
  panicking::expect_eq(q.pop_back(), Option{1});
  panicking::expect(!q.pop_back());
}

}  // namespace sfc::collections::test
