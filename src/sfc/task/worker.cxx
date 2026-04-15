#include "sfc/task.h"

#include "sfc/test/test.h"

namespace sfc::task::test {

SFC_TEST(worker) {
  int cnt = 0;

  auto worker = Worker{};
  auto thrd = thread::spawn([&]() { worker.run(); });

  auto add = [&](int val) { return [&, val]() { cnt += val; }; };

  sfc::expect_true(worker.post(Task::xnew(add(1))));
  worker.wait();
  sfc::expect_eq(cnt, 1);

  sfc::expect_true(worker.post(Task::xnew(add(2))));
  worker.wait();
  sfc::expect_eq(cnt, 3);

  sfc::expect_true(worker.post(Task::xnew(add(3))));
  worker.wait();
  sfc::expect_eq(cnt, 6);

  worker.stop();
}

}  // namespace sfc::task::test
