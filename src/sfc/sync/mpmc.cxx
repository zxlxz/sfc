#include "sfc/test/test.h"
#include "sfc/thread.h"
#include "sfc/sync/mpmc.h"
#include "sfc/io.h"

namespace sfc::sync::mpmc::test {

SFC_TEST(s1r1) {
  static const auto CNT = 100U;
  auto chan = Channel<u32>::with_capacity(16);

  auto recv_cnt = 0U;
  auto recv_sum = 0U;
  auto sender = [&]() {
    for (auto i = 0U; i < CNT; ++i) {
      sfc::assert_eq(chan.send(i), None{});
    }
  };

  auto receiver = [&]() {
    for (auto i = 0U; i < CNT; ++i) {
      auto val = chan.recv();
      sfc::assert_eq(val, Option{i});
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

SFC_TEST(s1r1_try) {
  auto chan = Channel<u32>::with_capacity(16);

  // try_recv returns empty when channel is empty
  {
    auto val = chan.try_recv();
    sfc::assert_eq(val, Option<u32>{});
  }

  // try_recv returns value when available
  {
    sfc::assert_eq(chan.send(7), None{});
    auto val = chan.try_recv();
    sfc::assert_eq(val, Option{7U});
  }

  // try_recv returns empty after drain
  {
    auto val = chan.try_recv();
    sfc::assert_eq(val, Option<u32>{});
  }
}

SFC_TEST(s2r1) {
  static const auto CNT = 50;
  auto chan = Channel<i32>::with_capacity(16);

  auto recv_cnt = 0U;
  auto recv_sum = 0;
  auto sender = [&](int k) {
    for (auto i = 0; i < CNT; ++i) {
      sfc::assert_eq(chan.send(k * i), None{});
    }
  };

  auto receiver = [&]() {
    for (auto i = 0; i < 2 * CNT; ++i) {
      auto val = chan.recv();
      sfc::assert_eq(val.is_some(), true);
      recv_cnt += 1;
      recv_sum += *val;
    }
  };

  {
    auto p1 = thread::spawn_joined([&]() { sender(+1); });
    auto p2 = thread::spawn_joined([&]() { sender(-1); });
    auto r1 = thread::spawn_joined([&]() { receiver(); });
  }

  sfc::assert_eq(recv_cnt, 2 * u32{CNT});
  sfc::assert_eq(recv_sum, 0);
}

SFC_TEST(s1r2) {
  static constexpr auto CNT = 50U;
  auto chan = Channel<u32>::with_capacity(16);

  auto recv_cnt = Atomic{0U};
  auto recv_sum = Atomic{0U};
  auto sender = [&]() {
    for (auto i = 0U; i < 2 * CNT; ++i) {
      sfc::assert_eq(chan.send(1U), None{});
    }
  };

  auto receiver = [&](unsigned limit) {
    for (auto i = 0U; i < limit; ++i) {
      auto val = chan.recv().unwrap();
      recv_sum.fetch_add(val);
      recv_cnt.fetch_add(1);
    }
  };

  {
    auto c1 = thread::spawn_joined([&]() { receiver(CNT); });
    auto c2 = thread::spawn_joined([&]() { receiver(CNT); });
    auto s1 = thread::spawn_joined([&]() { sender(); });
  }

  sfc::assert_eq(recv_cnt.load(), 2 * CNT);
  sfc::assert_eq(recv_sum.load(), 2 * CNT);
}

SFC_TEST(s2r2) {
  static const auto CNT = 50;
  auto chan = Channel<int>::with_capacity(16);

  auto recv_cnt = Atomic{0U};
  auto recv_sum = Atomic{0};
  auto sender = [&](int k) {
    for (auto i = 0; i < CNT; ++i) {
      sfc::assert_eq(chan.send(k * i), None{});
    }
  };

  auto receiver = [&]() {
    for (auto i = 0; i < CNT; ++i) {
      auto val = chan.recv();
      sfc::assert_eq(val.is_some(), true);
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

}  // namespace sfc::sync::mpmc::test
