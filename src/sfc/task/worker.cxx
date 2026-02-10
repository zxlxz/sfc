#include "sfc/task.h"

#include "sfc/test/test.h"

namespace sfc::task::test {

SFC_TEST(worker) {
  int cnt = 0;

  auto worker = Worker{};
  auto thrd = thread::spawn([&]() { worker.run(); });

  sfc::expect_true(worker.post(Task::xnew([&, val = 1]() { cnt += val; })));
  worker.wait();
  sfc::expect_eq(cnt, 1);

  sfc::expect_true(worker.post(Task::xnew([&, val = 2]() { cnt += val; })));
  worker.wait();
  sfc::expect_eq(cnt, 3);

  sfc::expect_true(worker.post(Task::xnew([&, val = 3]() { cnt += val; })));
  worker.wait();
  sfc::expect_eq(cnt, 6);

  worker.stop();
}

}  // namespace sfc::task::test
