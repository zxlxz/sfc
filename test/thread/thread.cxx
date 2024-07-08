#include "sfc/thread.h"

#include "sfc/io.h"
#include "sfc/test.h"

namespace sfc::thread {

SFC_TEST(current) {
  const auto thr = Thread::current();
  test::assert_ne(thr.id(), 0U);
  test::assert_eq(thr.name(), "sfc_test");
}

SFC_TEST(builder) {
  auto builder = Builder{};
  builder._name = String::from("thread_builder");
  builder._stack_size = 102400;
  builder.spawn(Box<void()>([]() {
    const auto thr = Thread::current();
    test::assert_eq(thr.name(), "thread_builder");
  }));
}

SFC_TEST(spawn) {
  auto n1 = 0;
  auto n2 = 0;

  {
    auto t1 = thread::spawn([&]() { n1 += 1; });
    auto t2 = thread::spawn([&]() { n2 += 1; });
  }

  test::assert_eq(n1, 1);
  test::assert_eq(n2, 1);
}

}  // namespace sfc::thread
