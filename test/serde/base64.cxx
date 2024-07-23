

#include "sfc/serde.h"
#include "sfc/test.h"
#include "sfc/io.h"

namespace sfc::serde::base64 {

SFC_TEST(encode_len) {
  u8 buf[16];
  test::assert_eq(encode({buf, 0}, {}), 0U);

  test::assert_eq(encode({buf, 1}, {}), 2U);
  test::assert_eq(encode({buf, 2}, {}), 3U);
  test::assert_eq(encode({buf, 3}, {}), 4U);

  test::assert_eq(encode({buf, 4}, {}), 4U + 2U);
  test::assert_eq(encode({buf, 5}, {}), 4U + 3U);
  test::assert_eq(encode({buf, 6}, {}), 4U + 4U);
}

SFC_TEST(decode_len) {
  u8 buf[16];
  test::assert_eq(decode({buf, 0}, {}), 0U);

  test::assert_eq(decode({buf, 1}, {}), 0U);
  test::assert_eq(decode({buf, 2}, {}), 1U);
  test::assert_eq(decode({buf, 3}, {}), 2U);
  test::assert_eq(decode({buf, 4}, {}), 3U);

  test::assert_eq(decode({buf, 5}, {}), 3U);
  test::assert_eq(decode({buf, 6}, {}), 3U + 1U);
  test::assert_eq(decode({buf, 7}, {}), 3U + 2U);
  test::assert_eq(decode({buf, 8}, {}), 3U + 3U);
}

SFC_TEST(encode_decode) {
  const auto str = Str{"test1234567"};
  const auto b64 = Str{"dGVzdDEyMzQ1Njc"};

  test::assert_eq(b64, String::from_u8(encode_vec(str.as_bytes())));
  test::assert_eq(str, String::from_u8(decode_vec(b64.as_bytes())));
}

}  // namespace sfc::serde::base64
