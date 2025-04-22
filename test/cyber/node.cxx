#include "sfc/cyber.h"
#include "sfc/io.h"
#include "sfc/log.h"
#include "sfc/test.h"
#include "sfc/thread.h"

namespace sfc::cyber {

class TestCmptA : CmptBase {
  cyber::Send<int> _send_a{"test.a"};

  int _idx = 0;

 public:
  auto on_init() -> bool override {
    return true;
  }

  void run() {
    io::println("node0.send: {+}", _idx);
    _send_a.send(_idx);
    _idx += 1;
  }
};

class TestCmptB : CmptBase {
  cyber::Recv<int> _recv_a{"test.a"};
  cyber::Send<int> _send_b{"test.b"};

 public:
  auto on_init() -> bool override {
    _recv_a.bind([&](const auto& val) { this->proc(val); });
    return true;
  }

  void on_shutdown() override {
    _recv_a.unbind();
  }

  void proc(int val) {
    io::println("node1.recv: {+}", val);
    _send_b.send(-val);
  }
};

class TestCmptC : CmptBase {
  cyber::Recv<int> _recv_b{"test.b"};

 public:
  auto on_init() -> bool override {
    _recv_b.bind([&](const auto& val) { this->proc(val); });
    return true;
  }

  void on_shutdown() override {
    _recv_b.unbind();
  }

  void proc(int val) {
    io::println("node2.recv: {+}", val);
  }
};

SFC_TEST(msg) {
  TestCmptA a;
  TestCmptB b;
  TestCmptC c;

  for (auto j = 0U; j < 4U; ++j) {
    a.run();
  }

  cyber::Sched::global().wait(time::Duration::from_millis(50));
}

}  // namespace sfc::cyber
