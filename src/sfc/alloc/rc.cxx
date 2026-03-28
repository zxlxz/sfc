#include "sfc/test/test.h"

namespace sfc::rc::test {

struct Foo {
  int* _cnt;

 public:
  Foo(int& cnt) : _cnt{&cnt} {
    *_cnt += 1;
  }

  ~Foo() {
    if (!_cnt) return;
    *_cnt -= 1;
  }

  Foo(Foo&& other) noexcept : _cnt{mem::take(other._cnt)} {}
};

SFC_TEST(own) {
  auto cnt = 0;

  {
    auto ra = Rc{Foo{cnt}};
    sfc::expect_eq(cnt, 1);

    auto rb = mem::move(ra);
    sfc::expect_eq(cnt, 1);
  }

  sfc::expect_eq(cnt, 0);
}

SFC_TEST(clone) {
  auto cnt = 0;

  {
    auto ra = Rc{Foo{cnt}};
    sfc::expect_eq(cnt, 1);

    auto rb = ra.clone();
    sfc::expect_eq(cnt, 1);
  }

  sfc::expect_eq(cnt, 0);
}

}  // namespace sfc::rc::test
