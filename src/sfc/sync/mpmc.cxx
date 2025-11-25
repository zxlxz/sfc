#include "sfc/sync/mpmc.h"

#include "sfc/test.h"

namespace sfc::sync::test {

SFC_TEST(spsc) {
  auto chan = Chan<int>{"test"};
  auto sender = Sender<int>{chan};
  auto recver = Receiver<int>{chan};

  for (auto i = 0; i < 10; ++i) {
    sender.send(42);
  }

  auto vals = Vec<int>{};
  while (auto msg = recver.recv()) {
    vals.push(*msg);
  }

  panicking::expect_eq(vals.len(), 10);
  for (auto i = 0; i < 10; ++i) {
    panicking::expect_eq(vals[i], 42);
  }
}

SFC_TEST(spmc) {
  auto chan = Chan<int>{"test"};
  auto sender = Sender<int>{chan};
  auto recver1 = Receiver<int>{chan};
  auto recver2 = Receiver<int>{chan};

  for (auto i = 0; i < 10; ++i) {
    sender.send(i);
  }

  auto vals1 = Vec<int>{};
  while (auto msg = recver1.recv()) {
    vals1.push(*msg);
  }

  auto vals2 = Vec<int>{};
  while (auto msg = recver2.recv()) {
    vals2.push(*msg);
  }

  panicking::expect_eq(vals1.len(), 10);
  panicking::expect_eq(vals2.len(), 10);

  for (auto i = 0; i < 10; ++i) {
    panicking::expect_eq(vals1[i], i);
    panicking::expect_eq(vals2[i], i);
  }
}

SFC_TEST(mpsc) {
  auto chan = Chan<int>{"test"};
  auto sender1 = Sender<int>{chan};
  auto sender2 = Sender<int>{chan};
  auto recver = Receiver<int>{chan};

  for (auto i = 0; i < 5; ++i) {
    sender1.send(2 * i + 0);
    sender2.send(2 * i + 1);
  }

  auto vals = Vec<int>{};
  while (auto msg = recver.recv()) {
    vals.push(*msg);
  }

  panicking::expect_eq(vals.len(), 10);
  for (auto i = 0; i < 10; ++i) {
    panicking::expect_eq(vals[i], i);
  }
}

SFC_TEST(mpmc) {
  auto chan = Chan<int>{"test"};
  auto sender1 = Sender<int>{chan};
  auto sender2 = Sender<int>{chan};
  auto recver1 = Receiver<int>{chan};
  auto recver2 = Receiver<int>{chan};

  for (auto i = 0; i < 5; ++i) {
    sender1.send(2 * i + 0);
    sender2.send(2 * i + 1);
  }

  auto vals1 = Vec<int>{};
  while (auto msg = recver1.recv()) {
    vals1.push(*msg);
  }

  auto vals2 = Vec<int>{};
  while (auto msg = recver2.recv()) {
    vals2.push(*msg);
  }

  panicking::expect_eq(vals1.len(), 10);
  panicking::expect_eq(vals2.len(), 10);

  for (auto i = 0; i < 10; ++i) {
    panicking::expect_eq(vals1[i], i);
    panicking::expect_eq(vals2[i], i);
  }
}

}  // namespace sfc::sync::test
