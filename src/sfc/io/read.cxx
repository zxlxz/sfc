#include "sfc/io.h"
#include "sfc/test/test.h"

namespace sfc::io::test {

SFC_TEST(read) {
  auto buf = Str{"0123456789"}.as_bytes();
  auto& r = trait::as_mut<io::Read>(buf);

  u8 tmp[4] = {};
  panicking::expect_eq(r.read(tmp).ok(), Option{4U});
  panicking::expect_eq(Str::from_utf8(tmp), "0123");

  panicking::expect_eq(r.read(tmp).ok(), Option{4U});
  panicking::expect_eq(Str::from_utf8(tmp), "4567");

  panicking::expect_eq(r.read(tmp).ok(), Option{2U});
  panicking::expect_eq(Str::from_utf8({tmp, 2}), "89");
}

SFC_TEST(read_exact) {
  auto buf = Str{"0123456789"}.as_bytes();
  auto& r = trait::as_mut<io::Read>(buf);

  u8 tmp[4] = {};
  panicking::expect(r.read_exact(tmp).is_ok());
  panicking::expect_eq(Str::from_utf8(tmp), "0123");

  panicking::expect(r.read_exact(tmp).is_ok());
  panicking::expect_eq(Str::from_utf8(tmp), "4567");

  panicking::expect(r.read_exact(tmp).is_err());
  panicking::expect_eq(Str::from_utf8({tmp, 2}), "89");
}

SFC_TEST(read_to_end) {
  auto buf = Str{"0123456789"}.as_bytes();
  auto& r = trait::as_mut<io::Read>(buf);

  auto v = Vec<u8>{};
  panicking::expect(r.read_to_end(v).is_ok());
  panicking::expect_eq(Str::from_utf8(v.as_slice()), "0123456789");
}

SFC_TEST(read_to_string) {
  auto buf = Str{"0123456789"}.as_bytes();
  auto& r = trait::as_mut<io::Read>(buf);

  auto str = String{};
  panicking::expect(r.read_to_string(str).is_ok());
  panicking::expect_eq(str, "0123456789");
}

}  // namespace sfc::io::test
