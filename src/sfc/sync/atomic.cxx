#include "sfc/test.h"
#include "sfc/sync/atomic.h"

namespace sfc::sync::test {

SFC_TEST(atomic_load_store) {
  {
    auto a0 = Atomic<int>{0};
    sfc::assert_eq(a0.load(), 0);
  }

  {
    auto a1 = Atomic<int>{1};
    sfc::assert_eq(a1.load(), 1);

    a1.store(2);
    sfc::assert_eq(a1.load(), 2);
  }
}

SFC_TEST(atomic_exchange) {
  auto a = Atomic<int>{1};
  sfc::assert_eq(a.exchange(2), 1);
  sfc::assert_eq(a.load(), 2);
}

SFC_TEST(atomic_compare_exchange) {
  auto a = Atomic<int>{1};
  sfc::assert_eq(a.compare_exchange(1, 2), true);
  sfc::assert_eq(a.load(), 2);

  sfc::assert_eq(a.compare_exchange(1, 3), false);
  sfc::assert_eq(a.load(), 2);
}

SFC_TEST(atomic_compare_exchange_weak) {
  auto a = Atomic<int>{1};

  // weak CAS may fail spuriously, so retry
  auto ok = false;
  for (auto i = 0; i < 100 && !ok; ++i) {
    ok = a.compare_exchange_weak(1, 2);
  }
  sfc::assert_eq(ok, true);
  sfc::assert_eq(a.load(), 2);

  sfc::assert_eq(a.compare_exchange_weak(1, 3), false);
  sfc::assert_eq(a.load(), 2);
}

SFC_TEST(atomic_fetch_add_sub) {
  auto a = Atomic<int>{1};
  sfc::assert_eq(a.fetch_add(2), 1);
  sfc::assert_eq(a.load(), 3);

  sfc::assert_eq(a.fetch_add<Ordering::Relaxed>(1), 3);
  sfc::assert_eq(a.load(), 4);

  sfc::assert_eq(a.fetch_sub(5), 4);
  sfc::assert_eq(a.load(), -1);
}

SFC_TEST(atomic_fence) {
  sync::fence();
  sync::fence<Ordering::Acquire>();
  sync::fence<Ordering::Release>();
}

}  // namespace sfc::sync::test
