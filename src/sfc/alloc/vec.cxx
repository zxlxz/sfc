#include "sfc/test.h"

namespace sfc::vec::test {

struct Element {
  int _value = 0;

 public:
  explicit Element(int value) : _value{value} {
    totalCnt() += 1;
  }

  ~Element() {
    _value = 0;
    totalCnt() -= 1;
  }

  Element(Element&& other) noexcept : _value{other._value} {
    other._value = 0;
    totalCnt() += 1;
  }

  static auto totalCnt() -> usize& {
    static usize val = 0U;
    return val;
  }
};

SFC_TEST(index) {
  int tmp[] = {0, 1, 2, 3};

  auto v = Vec<int>::from({tmp});
  panicking::expect_eq(v[0], 0);
  panicking::expect_eq(v[1], 1);
}

SFC_TEST(slice) {
  int tmp[] = {0, 1, 2, 3};

  auto v = Vec<int>::from({tmp});
  auto s = v.as_slice();
  panicking::expect_true(s.as_ptr() == v.as_ptr());
  panicking::expect_eq(s.len(), v.len());
}

SFC_TEST(clone) {
  int tmp[] = {0, 1, 2, 3};

  auto x = Vec<int>::from({tmp});
  auto y = x.clone();
  for (auto i = 0U; i < x.len(); ++i) {
    panicking::expect_eq(x[i], i);
    panicking::expect_eq(y[i], i);
  }
}

SFC_TEST(push) {
  int tmp[] = {0, 1, 2, 3};

  auto v = Vec<int>::from({tmp});
  v.push(4);
  v.push(5);
  panicking::expect_eq(v.len(), 6U);
  panicking::expect_eq(v.last(), Option{5});
}

SFC_TEST(pop) {
  int tmp[] = {0, 1, 2, 3};

  auto v = Vec<int>::from({tmp});
  panicking::expect_eq(*v.pop(), 3);
  panicking::expect_eq(*v.pop(), 2);
  panicking::expect_eq(*v.pop(), 1);
  panicking::expect_eq(*v.pop(), 0);
};

SFC_TEST(insert) {
  int tmp[] = {0, 1, 2, 3};

  auto v = Vec<int>::from({tmp});
  v.insert(1U, 1);
  v.insert(4U, 4);
  panicking::expect_eq(v.len(), 6U);
  panicking::expect_eq(v[0], 0);
  panicking::expect_eq(v[1], 1);
  panicking::expect_eq(v[2], 1);
  panicking::expect_eq(v[3], 2);
  panicking::expect_eq(v[4], 4);
  panicking::expect_eq(v[5], 3);
}

SFC_TEST(remove) {
  int tmp[] = {0, 1, 2, 3};

  auto v = Vec<int>::from({tmp});
  v.remove(1);
  panicking::expect_eq(v.len(), 3U);
  panicking::expect_eq(v[2], 3);
}

SFC_TEST(reserve) {
  Element::totalCnt() = 0U;

  auto v = Vec<Element>{};
  for (auto i = 0u; i < 4; ++i) {
    v.push(Element{static_cast<int>(i)});
  }
  panicking::expect_eq(Element::totalCnt(), 4U);

  for (auto i = 0u; i < 4; ++i) {
    panicking::expect_eq(v[i]._value, i);
  }

  v.reserve(10);
  panicking::expect_eq(Element::totalCnt(), 4U);
  for (auto i = 0U; i < 4U; ++i) {
    panicking::expect_eq(v[i]._value, i);
  }

  for (auto i = 4U; i < 100U; ++i) {
    v.push(Element{static_cast<int>(i)});
    panicking::expect_eq(v[i]._value, i);
  }
  panicking::expect_eq(Element::totalCnt(), 100U);

  for (auto i = 0U; i < 100U; ++i) {
    panicking::expect_eq(v[i]._value, i);
  }
}

}  // namespace sfc::vec::test
