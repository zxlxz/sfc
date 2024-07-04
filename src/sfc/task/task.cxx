#include "sfc/task.h"

#include "sfc/test.h"

namespace sfc::task::test {

SFC_TEST(worker) {
  int cnt = 0;

  auto worker = Worker{};
  panicking::assert_true(worker.submit([&, val = 1]() { cnt += val; }));
  worker.wait();
  panicking::assert_eq(cnt, 1);

  panicking::assert_true(worker.submit([&, val = 2]() { cnt += val; }));
  worker.wait();
  panicking::assert_eq(cnt, 3);

  panicking::assert_true(worker.submit([&, val = 3]() { cnt += val; }));
  worker.wait();
  panicking::assert_eq(cnt, 6);

  worker.stop();
}

}  // namespace sfc::thread::test
