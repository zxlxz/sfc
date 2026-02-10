#include "sfc/serde/base64.h"
#include "sfc/test/test.h"

namespace sfc::serde::base64::test {

SFC_TEST(encode) {
  const Str texts[] = {
      "",
      "Zg==",
      "Zm8=",
      "Zm9v",
      "Zm9vYg==",
      "Zm9vYmE=",
      "Zm9vYmFy",
  };

  const Str datas[] = {
      "",
      "f",
      "fo",
      "foo",
      "foob",
      "fooba",
      "foobar",
  };

  for (auto i = 0u; i < sizeof(texts) / sizeof(texts[0]); ++i) {
    const auto data = datas[i].as_bytes();
    const auto text = base64::encode(data);
    sfc::expect_eq(text, texts[i]);
  }
}

SFC_TEST(decode) {
  const Str texts[] = {
      "",
      "Zg==",
      "Zm8=",
      "Zm9v",
      "Zm9vYg==",
      "Zm9vYmE=",
      "Zm9vYmFy",
  };

  const Str datas[] = {
      "",
      "f",
      "fo",
      "foo",
      "foob",
      "fooba",
      "foobar",
  };

  for (auto i = 0u; i < sizeof(texts) / sizeof(texts[0]); ++i) {
    const auto text = texts[i];
    const auto data = base64::decode(text);
    sfc::expect_eq(Str::from_utf8(data.as_slice()), datas[i]);
  }
}

}  // namespace sfc::serde::base64::test
