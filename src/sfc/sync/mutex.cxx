#include <stdio.h>

#include "sfc/log.h"
#include "sfc/sync.h"
#include "sfc/test.h"
#include "sfc/thread.h"
namespace sfc::sync::test {

SFC_TEST(mutex) {
  static constexpr auto CNT = 10U;
  auto vec = Vec<int>::with_capacity(2 * CNT);
  auto mtx = Mutex{};

  auto push = [&](Str name) {
    for (auto i = 0u; i < CNT; ++i) {
      auto lock = mtx.lock();
      auto n = static_cast<int>(vec.len());
      vec.push(n);
      thread::sleep_ms(1);
      vec.push(n + 1);
      thread::sleep_ms(1);
    }
  };

  {
    auto t1 = thread::spawn([&]() { push("a"); });
    auto t2 = thread::spawn([&]() { push("b"); });
  }

  for (auto i = 0; i < vec.len(); ++i) {
    panicking::expect_eq(vec[i], i);
  }
}

SFC_TEST(reentrant_lock) {
  static constexpr auto CNT = 10U;
  auto vec = Vec<int>::with_capacity(4*CNT);
  auto mtx = ReentrantLock{};

  auto push = [&](Str name) {
    for (auto i = 0u; i < CNT; ++i) {
      auto lock1 = mtx.lock();
      auto n = static_cast<int>(vec.len());
      vec.push(n);
      thread::sleep_ms(1);
      auto lock2 = mtx.lock();
      vec.push(n + 1);
      thread::sleep_ms(1);
    }
  };

  {
    auto t1 = thread::spawn([&]() { push("a"); });
    auto t2 = thread::spawn([&]() { push("b"); });
  }

  for (auto i = 0; i < vec.len(); ++i) {
    panicking::expect_eq(vec[i], i);
  }
}

}  // namespace sfc::sync::test
