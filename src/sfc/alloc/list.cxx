#include "sfc/test/test.h"

namespace sfc::list::test {

struct RefCnt {
  int* _cnt;

 public:
  RefCnt(int& cnt) : _cnt{&cnt} {
    *_cnt += 1;
  }

  ~RefCnt() {
    if (!_cnt) return;
    *_cnt -= 1;
  }

  RefCnt(RefCnt&& other) noexcept : _cnt{other._cnt} {
    other._cnt = nullptr;
  }
};

SFC_TEST(index) {
  int tmp[] = {0, 1, 2, 3};

  auto v = List<int>::from(tmp);
  sfc::assert_eq(v[0], 0);
  sfc::assert_eq(v[1], 1);
}

SFC_TEST(slice) {
  int tmp[] = {0, 1, 2, 3};

  auto v = List<int>::from(tmp);
  auto s = v.as_slice();
  sfc::assert_eq(s.as_ptr(), v.as_ptr());
  sfc::assert_eq(s.len(), v.len());
}

SFC_TEST(clone) {
  u32 tmp[] = {0, 1, 2, 3};

  auto x = List<u32>::from(tmp);
  auto y = x.clone();
  for (auto i = 0U; i < x.len(); ++i) {
    sfc::assert_eq(x[i], i);
    sfc::assert_eq(y[i], i);
  }
}

SFC_TEST(push) {
  int tmp[] = {0, 1, 2, 3};

  auto v = List<int>::from(tmp);
  v.push(4);
  v.push(5);
  sfc::assert_eq(v.len(), 6U);
  sfc::assert_eq(v[v.len() - 1], 5);
}

SFC_TEST(pop) {
  int tmp[] = {0, 1, 2, 3};

  auto v = List<int>::from(tmp);
  sfc::assert_eq(v.pop().unwrap(), 3);
  sfc::assert_eq(v.pop().unwrap(), 2);
  sfc::assert_eq(v.pop().unwrap(), 1);
  sfc::assert_eq(v.pop().unwrap(), 0);
};

SFC_TEST(insert) {
  u32 tmp[] = {0, 1, 3};

  auto v = List<u32>::from(tmp);
  v.insert(2U, 2);
  v.insert(4U, 4);
  sfc::assert_eq(v.len(), 5U);
  for (auto i = 0U; i < 5U; ++i) {
    sfc::assert_eq(v[i], i);
  }
}

SFC_TEST(remove) {
  int tmp[] = {0, 1, 2, 3};

  auto v = List<int>::from(tmp);
  v.remove(1);
  sfc::assert_eq(v.len(), 3U);
  sfc::assert_eq(v[2], 3);
}

SFC_TEST(reserve) {
  auto v = List<int>::with_capacity(10);
  sfc::assert_eq(v.capacity(), 10U);
  for (auto i = 0; i < 10; ++i) {
    v.push(i);
  }
  v.reserve(1);
  sfc::assert_ge(v.capacity(), 16U);
}

SFC_TEST(drain) {
  int tmp[] = {0, 1, 2, 3, 4, 5};

  auto v = List<int>::from(tmp);
  v.drain(ops::Range{2, 4});
  sfc::assert_eq(v.len(), 4U);
  sfc::assert_eq(v[0], 0);
  sfc::assert_eq(v[1], 1);
  sfc::assert_eq(v[2], 4);
  sfc::assert_eq(v[3], 5);
}

SFC_TEST(memory) {
  auto cnt = 0;
  auto v = List<RefCnt>{};

  // push
  for (auto i = 0; i < 10; ++i) {
    v.push(RefCnt{cnt});
    sfc::assert_eq(cnt, i + 1);
  }

  // pop
  v.pop();
  sfc::assert_eq(cnt, 9);

  // insert
  v.insert(5, RefCnt{cnt});
  sfc::assert_eq(cnt, 10);

  // remove
  v.remove(5);
  sfc::assert_eq(cnt, 9);

  // drain
  v.drain(ops::Range{0, 5});
  sfc::assert_eq(cnt, 4);
}

SFC_TEST(retain) {
  // empty
  {
    auto v = List<int>{};
    v.retain([](int) { return false; });
    sfc::assert_eq(v.len(), 0U);
  }

  // normal
  {
    u32 tmp[] = {0, 1, 2, 3, 4, 5};
    auto v = List<u32>::from(tmp);
    v.retain([](u32 x) { return x % 2 == 0; });
    sfc::assert_eq(v.len(), 3U);
    for (auto i = 0U; i < v.len(); ++i) {
      sfc::assert_eq(v[i], i * 2);
    }
  }

  // all
  {
    u32 tmp[] = {0, 1, 2, 3, 4, 5};
    auto v = List<u32>::from(tmp);
    v.retain([](u32) { return true; });
    sfc::assert_eq(v.len(), 6U);
    for (auto i = 0U; i < v.len(); ++i) {
      sfc::assert_eq(v[i], i);
    }
  }

  // none
  {
    int tmp[] = {0, 1, 2, 3, 4, 5};
    auto v = List<int>::from(tmp);
    v.retain([](int) { return false; });
    sfc::assert_eq(v.len(), 0U);
  }
}

}  // namespace sfc::list::test
