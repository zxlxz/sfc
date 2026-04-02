#include "sfc/test/test.h"

namespace sfc::alloc::test {

SFC_TEST(alloc_basic) {
  Global a;
  const auto layout = Layout{16, 8};
  auto* ptr = a.alloc(layout);
  sfc::expect_ne(ptr, nullptr);
  a.dealloc(ptr, layout);
}

SFC_TEST(alloc_zero_size) {
  Global a;
  const auto layout = Layout{0, 8};
  auto* ptr = a.alloc(layout);
  sfc::expect_eq(ptr, nullptr);
}

SFC_TEST(realloc_nullptr) {
  Global a;
  const auto layout = Layout{16, 8};
  auto* ptr = a.realloc(nullptr, layout, 32);
  sfc::expect_ne(ptr, nullptr);
  a.dealloc(ptr, Layout{32, 8});
}

SFC_TEST(realloc_grow) {
  Global a;
  const auto layout = Layout{16, 8};
  auto* ptr = a.alloc(layout);
  sfc::expect_ne(ptr, nullptr);

  auto* new_ptr = a.realloc(ptr, layout, 32);
  sfc::expect_ne(new_ptr, nullptr);
  a.dealloc(new_ptr, Layout{32, 8});
}

SFC_TEST(realloc_shrink) {
  Global a;
  const auto layout = Layout{32, 8};
  auto* ptr = a.alloc(layout);
  sfc::expect_true(ptr != nullptr);

  auto* new_ptr = a.realloc(ptr, layout, 16);
  sfc::expect_ne(new_ptr, nullptr);
  a.dealloc(new_ptr, Layout{16, 8});
}

}  // namespace sfc::alloc::test
