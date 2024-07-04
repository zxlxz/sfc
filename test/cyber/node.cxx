#include "sfc/cyber.h"
#include "sfc/io.h"
#include "sfc/log.h"
#include "sfc/test.h"
#include "sfc/thread.h"

namespace sfc::cyber {

class TestNode0 {
  cyber::Send<int> _send;

  int _idx = 0;

 public:
  explicit TestNode0(Rc<Chan<int>> chan) : _send{mem::move(chan)} {}

  void run() {
    io::println("node0.send: {+}", _idx);
    _send.send(_idx);
    _idx += 1;
  }
};

class TestNode1 {
  cyber::Recv<int> _recv;
  cyber::Send<int> _send;

 public:
  TestNode1(Rc<Chan<int>> c1, Rc<Chan<int>> c2) : _recv{mem::move(c1)}, _send{mem::move(c2)} {
    _recv.bind([&](const auto& val) { this->proc(val); });
  }

  ~TestNode1() {
    io::println("node1.dtor");
  }

  void proc(int val) {
    io::println("node1.recv: {+}", val);
    _send.send(-val);
  }
};

class TestNode2 {
  cyber::Recv<int> _recv;

 public:
  explicit TestNode2(Rc<Chan<int>> chan) : _recv{mem::move(chan)} {
    _recv.bind([&](auto val) { this->proc(val); });
  }

  ~TestNode2() {
    io::println("node2.dtor");
  }

  void proc(int val) {
    io::println("node2.recv: {+}", val);
  }
};

SFC_TEST(msg) {
#if 0
  auto chan1 = Rc<Chan<int>>::xnew("chan1");
  auto chan2 = Rc<Chan<int>>::xnew("chan2");

  TestNode0 node0{chan1.clone()};
  TestNode1 node1{chan1.clone(), chan2.clone()};
  TestNode2 node2{chan2.clone()};

  for (auto j = 0U; j < 4U; ++j) {
    node0.run();
  }

  cyber::Sched::global().wait();
#endif
}

}  // namespace sfc::cyber
