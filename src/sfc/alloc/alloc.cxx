#include "sfc/test/test.h"

namespace sfc::alloc::test {

SFC_TEST(global_alloc_dealloc) {
  auto a = alloc::Global{};

  const auto layout = Layout::array<u64>(16);

  auto* ptr = a.allocate(layout);
  sfc::assert_ne(ptr, nullptr);

  a.deallocate(ptr, layout);
}

SFC_TEST(global_grow) {
  auto a = alloc::Global{};

  const auto layout = Layout::array<u64>(16);
  auto* ptr = a.allocate(layout);
  sfc::assert_ne(ptr, nullptr);

  auto* new_ptr = a.grow(ptr, layout, Layout::array<u64>(32));
  sfc::assert_ne(new_ptr, nullptr);
  a.deallocate(new_ptr, Layout::array<u64>(32));
}

SFC_TEST(global_shrink) {
  auto a = alloc::Global{};

  const auto layout = Layout::array<u64>(32);
  auto* ptr = a.allocate(layout);
  sfc::assert_ne(ptr, nullptr);

  auto* new_ptr = a.shrink(ptr, layout, Layout::array<u64>(16));
  sfc::assert_ne(new_ptr, nullptr);
  a.deallocate(new_ptr, Layout{}.array<u64>(16));
}

SFC_TEST(next_power_of_two) {
  sfc::assert_eq(next_power_of_two(0), 0U);
  sfc::assert_eq(next_power_of_two(1), 1U);
  sfc::assert_eq(next_power_of_two(2), 2U);
  sfc::assert_eq(next_power_of_two(3), 4U);
  sfc::assert_eq(next_power_of_two(17), 32U);
  sfc::assert_eq(next_power_of_two(1ULL << 40), 1ULL << 40);
  sfc::assert_eq(next_power_of_two((1ULL << 40) + 1), 1ULL << 41);
}

}  // namespace sfc::alloc::test
