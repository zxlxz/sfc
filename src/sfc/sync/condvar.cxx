#include "sfc/sync.h"
#include "sfc/test/test.h"
#include "sfc/thread.h"

namespace sfc::sync::test {

SFC_TEST(condvar_notify_one) {
  auto mtx = Mutex{};
  auto cv = Condvar{};
  auto ready = false;

  auto t1 = thread::spawn([&]() {
    auto lock = mtx.lock();
    while (!ready) {
      cv.wait(lock);
    }
  });

  thread::sleep_ms(5);

  {
    auto lock = mtx.lock();
    ready = true;
    cv.notify_one();
  }
}

SFC_TEST(condvar_notify_all) {
  static constexpr auto kThreadCnt = 3U;
  auto mtx = Mutex{};
  auto cv = Condvar{};
  auto ready = false;
  auto finished = 0U;

  auto threads = Vec<thread::JoinHandle>{};
  for (auto i = 0U; i < kThreadCnt; ++i) {
    threads.push(thread::spawn([&]() {
      auto lock = mtx.lock();
      while (!ready) {
        cv.wait(lock);
      }
      ++finished;
    }));
  }

  thread::sleep_ms(5);

  {
    auto lock = mtx.lock();
    ready = true;
    cv.notify_all();
  }

  thread::sleep_ms(5);

  auto lock = mtx.lock();
  sfc::expect_eq(finished, kThreadCnt);
}

SFC_TEST(condvar_wait_timeout) {
  auto mtx = Mutex{};
  auto cv = Condvar{};
  auto ready = false;

  {
    auto lock = mtx.lock();
    auto result = cv.wait_timeout(lock, time::Duration::from_millis(10));
    sfc::expect_false(result);
  }

  auto t1 = thread::spawn([&]() {
    auto lock = mtx.lock();
    while (!ready) {
      auto result = cv.wait_timeout(lock, time::Duration::from_millis(100));
      if (!result) {
        return;
      }
    }
  });

  thread::sleep_ms(5);

  {
    auto lock = mtx.lock();
    ready = true;
    cv.notify_one();
  }
}

SFC_TEST(condvar_wait_while) {
  auto mtx = Mutex{};
  auto cv = Condvar{};
  auto count = 0U;

  auto t1 = thread::spawn([&]() {
    auto lock = mtx.lock();
    cv.wait_while(lock, [&]() { return count < 5U; });
    sfc::expect_eq(count, 5U);
  });

  thread::sleep_ms(5);

  {
    auto lock = mtx.lock();
    for (auto i = 0U; i < 5U; ++i) {
      ++count;
      cv.notify_one();
      thread::sleep_ms(2);
    }
  }
}

SFC_TEST(condvar_producer_consumer) {
  static constexpr auto CNT = 10U;
  auto mtx = Mutex{};
  auto cv = Condvar{};

  auto vec = Vec<int>::with_capacity(CNT);
  auto produced = 0U;
  auto consumed = 0U;

  {
    auto producer = thread::spawn([&]() {
      for (auto i = 0; i < (int)CNT; ++i) {
        auto lock = mtx.lock();
        cv.wait_while(lock, [&]() { return vec.len() >= 5U; });
        vec.push(i);
        ++produced;
        cv.notify_one();
      }
    });

    auto consumer = thread::spawn([&]() {
      for (auto i = 0; i < (int)CNT; ++i) {
        auto lock = mtx.lock();
        cv.wait_while(lock, [&]() { return vec.is_empty(); });
        vec.remove(0);
        ++consumed;
        cv.notify_one();
      }
    });
  }

  sfc::expect_true(vec.is_empty());
  sfc::expect_eq(produced, CNT);
  sfc::expect_eq(consumed, CNT);
}

}  // namespace sfc::sync::test
