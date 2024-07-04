#include <stdio.h>

#include "sfc/log.h"
#include "sfc/sync.h"
#include "sfc/test.h"
#include "sfc/thread.h"
namespace sfc::sync::test {

SFC_TEST(mutex) {
  auto vec = Vec<int>::with_capacity(4 * 20);
  auto mtx = Mutex{};

  auto push = [&](Str name, usize n) {
    for (auto i = 0u; i < n; ++i) {
      auto lock = mtx.lock();
      auto n = static_cast<int>(vec.len());
      vec.push(n);
      thread::sleep_ms(2);
      vec.push(n + 1);
      thread::sleep_ms(2);
    }
  };

  {
    auto t1 = thread::spawn([&]() { push("a", 20); });
    auto t2 = thread::spawn([&]() { push("b", 20); });
  }

  for (auto i = 0; i < vec.len(); ++i) {
    panicking::assert_eq(vec[i], i);
  }
}

SFC_TEST(reentrant_lock) {
  auto vec = Vec<int>::with_capacity(4 * 20);
  auto mtx = ReentrantLock{};

  auto push = [&](Str name, usize n) {
    for (auto i = 0u; i < n; ++i) {
      auto lock1 = mtx.lock();
      auto n = static_cast<int>(vec.len());
      vec.push(n);
      thread::sleep_ms(2);
      auto lock2 = mtx.lock();
      vec.push(n + 1);
      thread::sleep_ms(2);
    }
  };

  {
    auto t1 = thread::spawn([&]() { push("a", 20); });
    auto t2 = thread::spawn([&]() { push("b", 20); });
  }

  for (auto i = 0; i < vec.len(); ++i) {
    panicking::assert_eq(vec[i], i);
  }
}

}  // namespace sfc::sync::test
