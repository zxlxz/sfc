#include "sfc/collections.h"
#include "sfc/test.h"

namespace sfc::collections::circbuf {

SFC_TEST(push_back) {
  CircBuf<int> buf{4};
  test::assert_eq(buf.len(), 0U);
  test::assert_true(buf.is_empty());
  test::assert_false(buf.is_full());

  buf.push_back(0);
  buf.push_back(1);
  buf.push_back(2);
  buf.push_back(3);

  test::assert_eq(buf.len(), 4U);

  test::assert_false(buf.is_empty());
  test::assert_true(buf.is_full());
  test::assert_eq(buf[0], 0);
  test::assert_eq(buf[1], 1);
  test::assert_eq(buf[2], 2);
  test::assert_eq(buf[3], 3);
}

SFC_TEST(push_front) {
  CircBuf<int> buf{4};
  test::assert_true(buf.is_empty());
  test::assert_false(buf.is_full());

  buf.push_front(0);
  buf.push_front(1);
  buf.push_front(2);
  buf.push_front(3);

  test::assert_eq(buf.len(), 4U);

  test::assert_false(buf.is_empty());
  test::assert_true(buf.is_full());
  test::assert_eq(buf[0], 3);
  test::assert_eq(buf[1], 2);
  test::assert_eq(buf[2], 1);
  test::assert_eq(buf[3], 0);
}

SFC_TEST(back_back) {
  CircBuf<int> buf{4};
  buf.push_back(0);
  buf.push_back(1);
  buf.push_back(2);
  buf.push_back(3);

  test::assert_eq(buf.pop_back(), Option{3});
  test::assert_eq(buf.pop_back(), Option{2});
  test::assert_eq(buf.pop_back(), Option{1});
  test::assert_eq(buf.pop_back(), Option{0});

  test::assert_true(buf.is_empty());
  test::assert_false(buf.is_full());
}

SFC_TEST(back_front) {
  CircBuf<int> buf{4};
  buf.push_back(0);
  buf.push_back(1);
  buf.push_back(2);
  buf.push_back(3);

  test::assert_eq(buf.pop_front(), Option{0});
  test::assert_eq(buf.pop_front(), Option{1});
  test::assert_eq(buf.pop_front(), Option{2});
  test::assert_eq(buf.pop_front(), Option{3});

  test::assert_true(buf.is_empty());
  test::assert_false(buf.is_full());
}

SFC_TEST(front_front) {
  CircBuf<int> buf{4};
  buf.push_front(0);
  buf.push_front(1);
  buf.push_front(2);
  buf.push_front(3);

  test::assert_eq(buf.pop_front(), Option{3});
  test::assert_eq(buf.pop_front(), Option{2});
  test::assert_eq(buf.pop_front(), Option{1});
  test::assert_eq(buf.pop_front(), Option{0});

  test::assert_true(buf.is_empty());
  test::assert_false(buf.is_full());
}

SFC_TEST(front_back) {
  CircBuf<int> buf{4};
  buf.push_front(0);
  buf.push_front(1);
  buf.push_front(2);
  buf.push_front(3);

  test::assert_eq(buf.pop_back(), Option{0});
  test::assert_eq(buf.pop_back(), Option{1});
  test::assert_eq(buf.pop_back(), Option{2});
  test::assert_eq(buf.pop_back(), Option{3});

  test::assert_true(buf.is_empty());
  test::assert_false(buf.is_full());
}

}  // namespace sfc::collections::circbuf
