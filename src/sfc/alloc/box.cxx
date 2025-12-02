#include "sfc/test.h"

namespace sfc::boxed::test {

struct Allocator {
  static auto& alloc_cnt() {
    static auto _new_cnt = 0U;
    return _new_cnt;
  }

  static auto& dealloc_cnt() {
    static auto _del_cnt = 0U;
    return _del_cnt;
  }

  static void reset() {
    alloc_cnt() = 0;
    dealloc_cnt() = 0;
  }

  static void* alloc(Layout layout) {
    ++alloc_cnt();
    return alloc::Global::alloc(layout);
  }

  static void dealloc(void* ptr, Layout layout) {
    ++dealloc_cnt();
    return alloc::Global::dealloc(ptr, layout);
  }
};

SFC_TEST(box_obj) {
  auto b1 = Box<int>::xnew(42);
  panicking::expect_eq(*b1, 42);

  auto b2 = Box<int>::xnew(43);
  panicking::expect_eq(*b2, 43);

  mem::swap(b1, b2);
  panicking::expect_eq(*b1, 43);
  panicking::expect_eq(*b2, 42);
}

SFC_TEST(box_fn) {
  auto f1 = [](int x) { return 10 * x; };

  auto b1 = Box<int(int)>::xnew(f1);
  panicking::expect_eq(b1(1), 10);

  auto b2 = mem::move(b1);
  panicking::expect_eq(b2(2), 20);
}

SFC_TEST(box_fn_copy) {
  int x1 = 1;
  int x2 = 2;

  auto f1 = [&x1](int x) { return 10 * x + x1; };
  auto f2 = [x1, x2](int x) { return 10 * x + x1 + x2; };
  auto f3 = [&x1, &x2](int x) { return 10 * x + x1 + x2; };

  {
    Allocator::reset();
    auto a1 = Box<int(int), Allocator>::xnew(f1);
    panicking::expect_eq(a1(1), 11);

    auto b2 = mem::move(a1);
    panicking::expect(!bool(a1));
  }
  panicking::expect_eq(Allocator::alloc_cnt(), 0);
  panicking::expect_eq(Allocator::dealloc_cnt(), 0);

  {
    auto a2 = Box<int(int), Allocator>::xnew(f2);
    panicking::expect_eq(a2(1), 13);

    auto b2 = mem::move(a2);
    panicking::expect_eq(b2(1), 13);
  }
  panicking::expect_eq(Allocator::alloc_cnt(), 0);
  panicking::expect_eq(Allocator::dealloc_cnt(), 0);

  {
    auto a3 = Box<int(int), Allocator>::xnew(f3);
    panicking::expect_eq(a3(1), 13);

    auto b3 = mem::move(a3);
    panicking::expect(!bool(a3));
    panicking::expect_eq(b3(1), 13);
  }
  panicking::expect_eq(Allocator::alloc_cnt(), 1);
  panicking::expect_eq(Allocator::dealloc_cnt(), 1);
}

struct IAdd {
  struct Meta {
    int (*_add)(void*, int, int) = nullptr;

    template <class X>
    static auto from(const X&) -> Meta {
      const auto add = [](void* p, int a, int b) { return static_cast<X*>(p)->add(a, b); };
      return {add};
    }
  };

  const Meta* _meta = nullptr;
  void* _self = nullptr;

 public:
  auto add(int a, int b) -> int {
    return (_meta->_add)(_self, a, b);
  }
};

struct XAdd {
  auto add(int a, int b) -> int {
    return a + b;
  }
};

SFC_TEST(box_ref) {
  auto b1 = Box<IAdd&>::xnew(XAdd{});
  panicking::expect_eq(b1->add(1, 2), 3);

  auto b2 = mem::move(b1);
  panicking::expect_eq(b2->add(3, 4), 7);
  panicking::expect(!bool(b1));
}

}  // namespace sfc::boxed::test
