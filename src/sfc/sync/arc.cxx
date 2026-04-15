#include "sfc/test/test.h"
#include "sfc/sync/arc.h"

namespace sfc::sync::test {

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

  Foo(const Foo& other) = delete;
  Foo& operator=(const Foo& other) = delete;
};

SFC_TEST(own) {
  auto cnt = 0;

  {
    auto ra = Arc<Foo>::xnew(cnt);
    sfc::expect_eq(cnt, 1);

    auto rb = mem::move(ra);
    sfc::expect_eq(cnt, 1);
  }

  sfc::expect_eq(cnt, 0);
}

SFC_TEST(clone) {
  auto cnt = 0;

  {
    const auto ra = Arc<Foo>::xnew(cnt);
    sfc::expect_eq(cnt, 1);

    const auto rb = ra.clone();
    sfc::expect_eq(cnt, 1);
  }

  sfc::expect_eq(cnt, 0);
}

}  // namespace sfc::sync::test
