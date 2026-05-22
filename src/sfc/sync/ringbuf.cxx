#include "sfc/test.h"
#include "sfc/thread.h"
#include "sfc/sync/ringbuf.h"

namespace sfc::sync::ringbuf::test {

SFC_TEST(push_pop_u32) {
  auto rb = RingBuf<u32>::with_capacity(4);

  const auto r0 = rb.pop();
  sfc::expect_eq(r0, None{});

  for (auto i = 0U; i < 4U; ++i) {
    sfc::expect_true(rb.push(i).is_ok());
  }

  for (auto i = 0U; i < 4U; ++i) {
    const auto val = rb.pop();
    sfc::expect_eq(val, Option{i});
  }

  sfc::expect_eq(rb.pop(), None{});
}

SFC_TEST(push_overflow) {
  auto rb = RingBuf<u32>::with_capacity(8);

  for (auto i = 0U; i < 8U; ++i) {
    sfc::expect_true(rb.push(i).is_ok());
  }

  sfc::expect_false(rb.push(99U).is_ok());
}

SFC_TEST(try_push_full) {
  auto rb = RingBuf<int>::with_capacity(2);
  (void)rb.push(1);
  (void)rb.push(2);

  int val = 3;
  sfc::expect_false(rb.try_push(val));  // full
  sfc::expect_eq(val, 3);               // val not consumed
}

SFC_TEST(fifo_order) {
  auto rb = RingBuf<int>::with_capacity(16);

  for (auto i = 0; i < 10; ++i) {
    (void)rb.push(i);
  }

  for (auto i = 0; i < 10; ++i) {
    const auto val = rb.pop();
    sfc::expect_eq(val, Option{i});
  }
}

SFC_TEST(interleaved_push_pop) {
  auto rb = RingBuf<int>::with_capacity(16);

  for (auto round = 0; round < 5; ++round) {
    for (auto i = 0; i < 3; ++i) {
      (void)rb.push(round * 3 + i);
    }

    for (auto i = 0; i < 3; ++i) {
      const auto val = rb.pop();
      sfc::expect_eq(val, Option{round * 3 + i});
    }
  }
}

SFC_TEST(move_semantics) {
  auto rb = RingBuf<String>::with_capacity(4);

  (void)rb.push(String::from("hello"));
  (void)rb.push(String::from("world"));

  const auto v0 = rb.pop();
  sfc::expect_eq(v0, Option{"hello"});

  const auto v1 = rb.pop();
  sfc::expect_eq(v1, Option{"world"});
}

SFC_TEST(move_ringbuf) {
  auto rb1 = RingBuf<int>::with_capacity(5);
  for (auto i = 0; i < 5; ++i) {
    (void)rb1.push(i);
  }

  auto rb2 = mem::move(rb1);
  sfc::expect_false(rb1.pop());

  for (auto i = 0; i < 5; ++i) {
    const auto val = rb2.pop();
    sfc::expect_eq(val, Option{i});
  }
}

SFC_TEST(clear) {
  auto rb = RingBuf<int>::with_capacity(5);
  for (auto i = 0; i < 5; ++i) {
    (void)rb.push(i);
  }

  rb.clear();
  sfc::expect_false(rb.pop());

  sfc::expect_true(rb.push(42).is_ok());
  const auto val = rb.pop();
  sfc::expect_eq(val, Option{42});
}

SFC_TEST(mpmc_s1r1) {
  static const auto CNT = 100U;
  auto rb = RingBuf<u32>::with_capacity(100);

  auto recv_cnt = 0U;
  auto recv_sum = 0U;

  auto sender = [&]() {
    for (auto i = 0U; i < CNT; ++i) {
      while (!rb.push(i).is_ok()) {
        thread::yield_now();
      }
    }
  };

  auto receiver = [&]() {
    for (auto i = 0U; i < CNT; ++i) {
      auto val = rb.pop();
      while (!val) {
        thread::yield_now();
        val = rb.pop();
      }
      recv_cnt += 1;
      recv_sum += *val;
    }
  };

  {
    auto s1 = thread::spawn([&]() { sender(); });
    auto r1 = thread::spawn([&]() { receiver(); });
  }

  sfc::expect_eq(recv_cnt, CNT);
  sfc::expect_eq(recv_sum, CNT * (CNT - 1) / 2);
}

SFC_TEST(mpmc_s2r2) {
  static const auto CNT = 50;
  auto rb = RingBuf<int>::with_capacity(100);

  auto recv_cnt = Atomic{0U};
  auto recv_sum = Atomic{0};

  auto sender = [&](int k) {
    for (auto i = 0; i < CNT; ++i) {
      while (!rb.push(k * i).is_ok()) {
        thread::yield_now();
      }
    }
  };

  auto receiver = [&]() {
    for (auto i = 0; i < CNT; ++i) {
      auto val = rb.pop();
      while (!val) {
        thread::yield_now();
        val = rb.pop();
      }
      recv_cnt.fetch_add(1);
      recv_sum.fetch_add(*val);
    }
  };

  {
    auto p1 = thread::spawn([&]() { sender(+1); });
    auto p2 = thread::spawn([&]() { sender(-1); });
    auto c1 = thread::spawn([&]() { receiver(); });
    auto c2 = thread::spawn([&]() { receiver(); });
  }

  sfc::expect_eq(recv_cnt.load(), 2 * CNT);
  sfc::expect_eq(recv_sum.load(), 0);
}

}  // namespace sfc::sync::ringbuf::test
