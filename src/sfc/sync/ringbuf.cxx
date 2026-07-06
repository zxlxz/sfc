#include "sfc/test.h"
#include "sfc/thread.h"
#include "sfc/sync/ringbuf.h"

namespace sfc::sync::ringbuf::test {

SFC_TEST(base) {
  auto mq = RingBuf<u32>::with_capacity(4);

  const auto r0 = mq.pop();
  sfc::assert_eq(r0, None{});

  for (auto i = 0U; i < 4U; ++i) {
    sfc::assert_eq(mq.push(i), None{});
  }

  for (auto i = 0U; i < 4U; ++i) {
    const auto val = mq.pop();
    sfc::assert_eq(val, Option{i});
  }

  sfc::assert_eq(mq.pop(), None{});
}

SFC_TEST(push_full) {
  const auto cap = 4U;
  auto mq = RingBuf<u32>::with_capacity(cap);
  for (auto i = 0U; i < cap; ++i) {
    sfc::assert_eq(mq.push(i), None{});
  }
  sfc::assert_eq(mq.is_empty(), false);
  sfc::assert_eq(mq.is_full(), true);

  auto val = cap + 1;
  sfc::assert_eq(mq.push(val), Option{cap + 1});  // full
  sfc::assert_eq(mq.is_full(), true);

  for (auto i = 0U; i < cap; ++i) {
    const auto val = mq.pop();
    sfc::assert_eq(val, Option{i});

    sfc::assert_eq(mq.is_full(), false);
    sfc::assert_eq(mq.push(cap + i), None{});
    sfc::assert_eq(mq.is_full(), true);
  }

  for (auto i = 0U; i < cap; ++i) {
    sfc::assert_eq(mq.pop(), Option{cap + i});
  }
}

SFC_TEST(clear) {
  auto mq = RingBuf<int>::with_capacity(5);
  for (auto i = 0; i < 5; ++i) {
    (void)mq.push(i);
  }

  mq.clear();
  sfc::assert_eq(mq.pop(), None{});

  sfc::assert_eq(mq.push(42), None{});
  const auto val = mq.pop();
  sfc::assert_eq(val, Option{42});
}

SFC_TEST(fifo_order) {
  auto mq = RingBuf<int>::with_capacity(16);

  for (auto i = 0; i < 10; ++i) {
    (void)mq.push(i);
  }

  for (auto i = 0; i < 10; ++i) {
    const auto val = mq.pop();
    sfc::assert_eq(val, Option{i});
  }
}

SFC_TEST(mpmc_s1r1) {
  static const auto CNT = 100U;
  auto mq = RingBuf<u32>::with_capacity(100);

  auto recv_cnt = 0U;
  auto recv_sum = 0U;

  auto sender = [&]() {
    for (auto i = 0U; i < CNT; ++i) {
      while (mq.push(i).is_some()) {
        thread::yield_now();
      }
    }
  };

  auto receiver = [&]() {
    for (auto i = 0U; i < CNT; ++i) {
      auto val = mq.pop();
      while (!val) {
        thread::yield_now();
        val = mq.pop();
      }
      recv_cnt += 1;
      recv_sum += *val;
    }
  };

  {
    auto s1 = thread::spawn_joined([&]() { sender(); });
    auto r1 = thread::spawn_joined([&]() { receiver(); });
  }

  sfc::assert_eq(recv_cnt, CNT);
  sfc::assert_eq(recv_sum, CNT * (CNT - 1) / 2);
}

SFC_TEST(mpmc_s2r2) {
  static const auto CNT = 50;
  auto mq = RingBuf<int>::with_capacity(100);

  auto recv_cnt = Atomic{0U};
  auto recv_sum = Atomic{0};

  auto sender = [&](int k) {
    for (auto i = 0; i < CNT; ++i) {
      while (mq.push(k * i).is_some()) {
        thread::yield_now();
      }
    }
  };

  auto receiver = [&]() {
    for (auto i = 0; i < CNT; ++i) {
      auto val = mq.pop();
      while (!val) {
        thread::yield_now();
        val = mq.pop();
      }
      recv_cnt.fetch_add(1);
      recv_sum.fetch_add(*val);
    }
  };

  {
    auto p1 = thread::spawn_joined([&]() { sender(+1); });
    auto p2 = thread::spawn_joined([&]() { sender(-1); });
    auto c1 = thread::spawn_joined([&]() { receiver(); });
    auto c2 = thread::spawn_joined([&]() { receiver(); });
  }

  sfc::assert_eq(recv_cnt.load(), 2 * u32{CNT});
  sfc::assert_eq(recv_sum.load(), 0);
}

}  // namespace sfc::sync::queue::test
