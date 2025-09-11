#include "sfc/task.h"

#include "sfc/test.h"

namespace sfc::task::test {

SFC_TEST(worker) {
  int cnt = 0;

  auto worker = Worker{};
  panicking::expect_true(worker.submit([&, val = 1]() { cnt += val; }));
  worker.wait();
  panicking::expect_eq(cnt, 1);

  panicking::expect_true(worker.submit([&, val = 2]() { cnt += val; }));
  worker.wait();
  panicking::expect_eq(cnt, 3);

  panicking::expect_true(worker.submit([&, val = 3]() { cnt += val; }));
  worker.wait();
  panicking::expect_eq(cnt, 6);

  worker.stop();
}

}  // namespace sfc::thread::test
