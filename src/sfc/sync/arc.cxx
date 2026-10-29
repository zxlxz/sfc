#include "sfc/test/test.h"
#include "sfc/thread.h"
#include "sfc/sync/arc.h"

namespace sfc::sync::test {

struct Cnter {
  int* _cnt;

 public:
  Cnter(int& cnt) : _cnt{&cnt} {
    *_cnt += 1;
  }

  ~Cnter() {
    if (!_cnt) return;
    *_cnt -= 1;
  }

  Cnter(const Cnter& other) = delete;
  Cnter& operator=(const Cnter& other) = delete;
};

SFC_TEST(own) {
  auto cnt = 0;

  {
    auto ra = Arc<Cnter>::new_(cnt);
    sfc::assert_eq(cnt, 1);

    auto rb = mem::move(ra);
    sfc::assert_eq(cnt, 1);
  }

  sfc::assert_eq(cnt, 0);
}

SFC_TEST(clone) {
  auto cnt = 0;

  {
    const auto ra = Arc<Cnter>::new_(cnt);
    sfc::assert_eq(cnt, 1);

    const auto rb = ra.clone();
    sfc::assert_eq(cnt, 1);
  }

  sfc::assert_eq(cnt, 0);
}

SFC_TEST(cross_thread) {
  struct Shared {
    sync::Atomic<u32> _sum{0};
    u32* _observe;
    int* _cnt;

   public:
    Shared(u32* observe, int& cnt) : _observe{observe}, _cnt{&cnt} {
      *_cnt += 1;
    }

    ~Shared() {
      if (!_cnt) return;
      *_observe = _sum.load();
      *_cnt -= 1;
    }
  };

  constexpr auto THREADS = u32{4};
  const auto total = THREADS * (THREADS + 1) / 2;

  auto cnt = 0;
  auto observe = u32{0};

  {
    auto ra = Arc<Shared>::new_(&observe, cnt);
    sfc::assert_eq(cnt, 1);

    {
      auto scope = thread::Scope{};
      for (auto i = u32{1}; i <= THREADS; ++i) {
        scope.spawn([&, i]() {
          auto rb = ra.clone();
          rb->_sum.fetch_add(i);
        });
      }
    }  // joins all workers, dropping every clone

    sfc::assert_eq(cnt, 1);  // only the original Arc remains
    sfc::assert_eq(ra->_sum.load(), total);
  }  // last drop: destroys Shared, publishing the final sum to observe

  sfc::assert_eq(observe, total);
  sfc::assert_eq(cnt, 0);
}

}  // namespace sfc::sync::test
