#include "sfc/test/test.h"
#include "sfc/thread.h"
#include "sfc/sync/mpmc.h"

namespace sfc::sync::mpmc::test {

SFC_TEST(s1r1) {
  static const auto CNT = 100U;
  auto chan = Channel<u32>{};

  auto recv_cnt = 0U;
  auto recv_sum = 0U;
  auto sender = [&]() {
    for (auto i = 0U; i < CNT; ++i) {
      chan.send(static_cast<u32>(i));
    }
  };

  auto receiver = [&]() {
    for (auto i = 0U; i < CNT; ++i) {
      auto val = chan.recv();
      sfc::expect_eq(val, static_cast<u32>(i));
      recv_cnt += 1;
      recv_sum += val;
    }
  };

  {
    auto s1 = thread::spawn([&]() { sender(); });
    auto r1 = thread::spawn([&]() { receiver(); });
  }

  sfc::expect_eq(recv_cnt, CNT);
  sfc::expect_eq(recv_sum, CNT * (CNT - 1) / 2);
}

SFC_TEST(s1r1_timeout) {
  auto chan = Channel<u32>{};

  // recv_timeout returns empty when channel is empty
  {
    auto result = chan.recv_timeout(time::Duration::from_millis(5));
    sfc::expect_eq(result, Option<u32>{});
  }

  // recv_timeout returns value when available immediately
  {
    chan.send(3);
    const auto val = chan.recv_timeout(time::Duration::from_millis(100));
    sfc::expect_eq(val, Option{3U});
  }

  // recv_timeout used in threaded producer-receiver
  static constexpr auto CNT = 20U;
  auto recv_cnt = 0U;
  auto recv_sum = 0U;
  auto sender = [&]() {
    for (auto i = 0U; i < CNT; ++i) {
      chan.send(static_cast<u32>(i));
    }
  };

  auto receiver = [&]() {
    for (auto i = 0U; i < CNT; ++i) {
      auto val = chan.recv_timeout(time::Duration::from_millis(1000));
      sfc::expect_eq(val, Option{i});
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

SFC_TEST(s1r1_try) {
  auto chan = Channel<u32>{};

  // try_recv returns empty when channel is empty
  {
    auto val = chan.try_recv();
    sfc::expect_eq(val, Option<u32>{});
  }

  // try_recv returns value when available
  {
    chan.send(7);
    auto val = chan.try_recv();
    sfc::expect_eq(val, Option{7U});
  }

  // try_recv returns empty after drain
  {
    auto val = chan.try_recv();
    sfc::expect_eq(val, Option<u32>{});
  }
}

SFC_TEST(s2r1) {
  static const auto CNT = 50U;
  auto chan = Channel<int>{};

  auto recv_cnt = 0U;
  auto recv_sum = 0;
  auto sender = [&](int k) {
    for (auto i = 0U; i < CNT; ++i) {
      chan.send(static_cast<int>(k * i));
    }
  };

  auto receiver = [&]() {
    for (auto i = 0U; i < 2 * CNT; ++i) {
      auto val = chan.recv();
      recv_cnt += 1;
      recv_sum += val;
    }
  };

  {
    auto p1 = thread::spawn([&]() { sender(+1); });
    auto p2 = thread::spawn([&]() { sender(-1); });
    auto r1 = thread::spawn([&]() { receiver(); });
  }

  sfc::expect_eq(recv_cnt, 2 * CNT);
  sfc::expect_eq(recv_sum, 0);
}

SFC_TEST(s1r2) {
  static constexpr auto CNT = 50U;
  auto chan = Channel<int>{};

  auto recv_cnt = Atomic{0U};
  auto recv_sum = Atomic{0U};
  auto sender = [&]() {
    for (auto i = 0U; i < 2 * CNT; ++i) {
      chan.send(1U);
    }
  };

  auto receiver = [&](unsigned limit) {
    for (auto i = 0U; i < limit; ++i) {
      auto val = chan.recv();
      recv_sum.fetch_add(val);
      recv_cnt.fetch_add(1);
    }
  };

  {
    auto c1 = thread::spawn([&]() { receiver(CNT); });
    auto c2 = thread::spawn([&]() { receiver(CNT); });
    auto s1 = thread::spawn([&]() { sender(); });
  }

  sfc::expect_eq(recv_cnt.load(), 2 * CNT);
  sfc::expect_eq(recv_sum.load(), 2 * CNT);
}

SFC_TEST(s2r2) {
  static const auto CNT = 50U;
  auto chan = Channel<int>{};

  auto recv_cnt = Atomic{0U};
  auto recv_sum = Atomic{0};
  auto sender = [&](int k) {
    for (auto i = 0U; i < CNT; ++i) {
      chan.send(static_cast<int>(k * i));
    }
  };

  auto receiver = [&]() {
    for (auto i = 0U; i < CNT; ++i) {
      auto val = chan.recv();
      recv_cnt.fetch_add(1);
      recv_sum.fetch_add(val);
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

}  // namespace sfc::sync::mpmc::test
