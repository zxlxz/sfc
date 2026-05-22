#include "sfc/test/test.h"
#include "sfc/log.h"
#include "sfc/thread.h"
#include "sfc/sync/mutex.h"

namespace sfc::sync::test {

SFC_TEST(mutex) {
  static constexpr auto CNT = 10U;
  auto list = List<usize>::with_capacity(2 * CNT);
  auto mtx = Mutex{};

  auto thread_func = [&]() {
    for (auto i = 0U; i < CNT; ++i) {
      auto lock = mtx.lock();
      const auto n = list.len();
      list.push(n);
      thread::sleep_ms(1);
      list.push(n + 1);
      thread::sleep_ms(1);
    }
  };

  {
    auto t1 = thread::spawn([&]() { thread_func(); });
    auto t2 = thread::spawn([&]() { thread_func(); });
  }

  for (auto i = 0U; i < list.len(); ++i) {
    sfc::expect_eq(list[i], i);
  }
}

SFC_TEST(mutex_try_lock) {
  auto mtx = Mutex{};

  {
    auto guard = mtx.try_lock();
    sfc::expect_true(guard.is_some());
  }

  {
    auto guard1 = mtx.try_lock();
    sfc::expect_true(guard1.is_some());

    auto guard2 = mtx.try_lock();
    sfc::expect_false(guard2.is_some());
  }
}

SFC_TEST(reentrant_lock) {
  static constexpr auto CNT = 10U;
  auto list = List<usize>::with_capacity(4 * CNT);
  auto mtx = ReentrantLock{};

  auto push = [&](Str name) {
    for (auto i = 0u; i < CNT; ++i) {
      auto lock1 = mtx.lock();
      const auto n = list.len();
      list.push(n);
      thread::sleep_ms(1);
      auto lock2 = mtx.lock();
      list.push(n + 1);
      thread::sleep_ms(1);
    }
  };

  {
    auto t1 = thread::spawn([&]() { push("a"); });
    auto t2 = thread::spawn([&]() { push("b"); });
  }

  for (auto i = 0UL; i < list.len(); ++i) {
    sfc::expect_eq(list[i], i);
  }
}

}  // namespace sfc::sync::test
