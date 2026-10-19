#include "sfc/test.h"

namespace sfc::mem::test {

SFC_TEST(layout_of) {
  using A = u32;
  sfc::assert_eq(Layout::of<A>().size, sizeof(A));
  sfc::assert_eq(Layout::of<A>().align, alignof(A));

  using B = u64;
  sfc::assert_eq(Layout::of<B>().size, sizeof(B));
  sfc::assert_eq(Layout::of<B>().align, alignof(B));

  using C = Tuple<u32, u64>;
  sfc::assert_eq(Layout::of<C>().size, sizeof(C));
  sfc::assert_eq(Layout::of<C>().align, alignof(C));
}

SFC_TEST(layout_for_value) {
  const auto a = 0U;
  sfc::assert_eq(Layout::for_value(a).size, sizeof(u32));
  sfc::assert_eq(Layout::for_value(a).align, alignof(u32));

  const auto b = 0ULL;
  sfc::assert_eq(Layout::for_value(b).size, sizeof(u64));
  sfc::assert_eq(Layout::for_value(b).align, alignof(u64));

  const auto c = Tuple{0U, 0ULL};
  sfc::assert_eq(Layout::for_value(c).size, sizeof(u64) * 2);
  sfc::assert_eq(Layout::for_value(c).align, alignof(u64));
}

SFC_TEST(layout_array) {
  sfc::assert_eq(Layout::array<u32>(3).size, sizeof(u32) * 3);
  sfc::assert_eq(Layout::array<u32>(3).align, alignof(u32));

  sfc::assert_eq(Layout::array<u64>(5).size, sizeof(u64) * 5);
  sfc::assert_eq(Layout::array<u64>(5).align, alignof(u64));
}

SFC_TEST(move) {
  auto a = String::from("a");
  sfc::assert_eq(a, "a");

  auto b = mem::move(a);
  sfc::assert_eq(b, "a");
}

SFC_TEST(swap) {
  auto a = String::from("a");
  auto b = String::from("b");

  mem::swap(a, b);
  sfc::assert_eq(a, "b");
  sfc::assert_eq(b, "a");
}

struct RefCount {
  int* _cnt;

 public:
  RefCount(int& cnt) noexcept : _cnt{&cnt} {
    *_cnt += 1;
  }

  ~RefCount() noexcept {
    if (_cnt == nullptr) return;
    *_cnt -= 1;
  }

  RefCount(RefCount&& other) noexcept : _cnt{other._cnt} {
    other._cnt = nullptr;
  }
};

SFC_TEST(assign) {
  auto a = String::from("a");
  sfc::assert_eq(a, "a");

  mem::assign(a, String::from("b"));
  sfc::assert_eq(a, "b");
}

SFC_TEST(replace) {
  auto a = String::from("a");
  sfc::assert_eq(a, "a");

  auto b = mem::replace(a, String::from("b"));
  sfc::assert_eq(a, "b");
  sfc::assert_eq(b, "a");
}

SFC_TEST(take) {
  auto a = 2;
  sfc::assert_eq(a, 2);

  auto b = mem::take(a);
  sfc::assert_eq(a, 0);
  sfc::assert_eq(b, 2);
}

}  // namespace sfc::mem::test
