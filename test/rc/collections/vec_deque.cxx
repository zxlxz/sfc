#include "rc/test.h"

#include "rc/collections/vec_deque.h"

namespace rc::collections::vec_deque {

rc_test(push) {
  auto q = VecDeque<i32>{};
  q.push_front(2);
  q.push_front(1);
  q.push_front(0);

  q.push_back(3);
  q.push_back(4);
  q.push_back(5);

  log::info("q = {}", q);
}

}  // namespace rc::collections::vec_deque
