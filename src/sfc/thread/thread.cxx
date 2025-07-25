#include "sfc/thread.h"

#include "sfc/io.h"
#include "sfc/test.h"

namespace sfc::thread {

SFC_TEST(builder) {
  const auto builder = Builder{
    .name = "thread_builder",
  };
  
  builder.spawn(Box<void()>::xnew([]() {
    const auto thr = Thread::current();
    panicking::assert_eq(thr.name(), "thread_builder");
  }));
}

SFC_TEST(spawn) {
  auto n1 = 0;
  auto n2 = 0;

  {
    auto t1 = thread::spawn([&]() { n1 += 1; });
    auto t2 = thread::spawn([&]() { n2 += 1; });
  }

  panicking::assert_eq(n1, 1);
  panicking::assert_eq(n2, 1);
}

}  // namespace sfc::thread
