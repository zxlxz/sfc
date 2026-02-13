#include "sfc/test/test.h"

namespace sfc::alloc::test {

SFC_TEST(alloc_basic) {
  Global a;
  const auto layout = Layout{16, 8};
  auto* ptr = a.alloc(layout);
  sfc::expect_true(ptr != nullptr);
  a.dealloc(ptr, layout);
}

SFC_TEST(alloc_zero_size) {
  Global a;
  const auto layout = Layout{0, 8};
  auto* ptr = a.alloc(layout);
  sfc::expect_eq(ptr, nullptr);
}

SFC_TEST(alloc_array_int) {
  Global a;
  auto* ptr = a.alloc_array<int>(10);
  sfc::expect_true(ptr != nullptr);
  a.dealloc_array(ptr, 10);
}

SFC_TEST(realloc_nullptr) {
  Global a;
  const auto layout = Layout{16, 8};
  auto* ptr = a.realloc(nullptr, layout, 32);
  sfc::expect_true(ptr != nullptr);
  a.dealloc(ptr, Layout{32, 8});
}

SFC_TEST(realloc_grow) {
  Global a;
  const auto layout = Layout{16, 8};
  auto* ptr = a.alloc(layout);
  sfc::expect_true(ptr != nullptr);

  auto* new_ptr = a.realloc(ptr, layout, 32);
  sfc::expect_true(new_ptr != nullptr);
  a.dealloc(new_ptr, Layout{32, 8});
}

SFC_TEST(realloc_shrink) {
  Global a;
  const auto layout = Layout{32, 8};
  auto* ptr = a.alloc(layout);
  sfc::expect_true(ptr != nullptr);

  auto* new_ptr = a.realloc(ptr, layout, 16);
  sfc::expect_true(new_ptr != nullptr);
  a.dealloc(new_ptr, Layout{16, 8});
}

SFC_TEST(realloc_array_trivial) {
  Global a;
  auto* ptr = a.alloc_array<int>(4);
  sfc::expect_true(ptr != nullptr);

  for (int i = 0; i < 4; ++i) {
    ptr[i] = i;
  }

  auto* new_ptr = a.realloc_array(ptr, 4, 8, 4);
  sfc::expect_true(new_ptr != nullptr);
  sfc::expect_eq(new_ptr[0], 0);
  sfc::expect_eq(new_ptr[1], 1);
  sfc::expect_eq(new_ptr[2], 2);
  sfc::expect_eq(new_ptr[3], 3);

  a.dealloc_array(new_ptr, 8);
}

SFC_TEST(realloc_array_shrink_trivial) {
  Global a;
  auto* ptr = a.alloc_array<int>(8);
  sfc::expect_true(ptr != nullptr);

  for (int i = 0; i < 8; ++i) {
    ptr[i] = i;
  }

  auto* new_ptr = a.realloc_array(ptr, 8, 4, 8);
  sfc::expect_true(new_ptr != nullptr);
  sfc::expect_eq(new_ptr[0], 0);
  sfc::expect_eq(new_ptr[1], 1);
  sfc::expect_eq(new_ptr[2], 2);
  sfc::expect_eq(new_ptr[3], 3);

  a.dealloc_array(new_ptr, 4);
}

}  // namespace sfc::alloc::test
