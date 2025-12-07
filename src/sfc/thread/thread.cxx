#include "sfc/thread.h"

#include "sfc/io.h"
#include "sfc/test/ut.h"

namespace sfc::thread::test {

SFC_TEST(spawn) {
  auto n1 = 0;
  auto n2 = 0;

  {
    auto t1 = thread::spawn([&]() { n1 += 1; });
    auto t2 = thread::spawn([&]() { n2 += 1; });
  }

  panicking::expect_eq(n1, 1);
  panicking::expect_eq(n2, 1);
}

}  // namespace sfc::thread
