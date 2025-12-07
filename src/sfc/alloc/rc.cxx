#include "sfc/test/ut.h"

namespace sfc::rc::test {

struct Foo {
  int _val;

 public:
  static auto obj_cnt() -> int& {
    static auto res = 0;
    return res;
  }

  explicit Foo(int value) : _val{value} {
    obj_cnt() += 1;
  }

  ~Foo() {
    obj_cnt() -= 1;
  }

  Foo(const Foo&) = delete;
};

SFC_TEST(rc) {
  Foo::obj_cnt() = 0;
  {
    auto a = Rc<Foo>::xnew(1);
    panicking::expect_eq(a->_val, 1);
    panicking::expect_eq(Foo::obj_cnt(), 1);

    {
      auto b = a.clone();
      panicking::expect_eq(b->_val, 1);
      panicking::expect_eq(Foo::obj_cnt(), 1);
    }

    panicking::expect_eq(Foo::obj_cnt(), 1);
  }

  panicking::expect_eq(Foo::obj_cnt(), 0);
}

}  // namespace sfc::rc::test
