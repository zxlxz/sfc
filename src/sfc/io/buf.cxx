#include "sfc/io/buf.h"

#include "sfc/test/test.h"

namespace sfc::io::test {

SFC_TEST(buf_read) {
  const auto in = Str{"0123456789"};
  auto r = BufReader{in.as_bytes()};

  // peak
  {
    const auto s1 = r.peak(1).unwrap_or({});
    sfc::expect_eq(Str::from_utf8(s1), "0");

    const auto s10 = r.peak(10).unwrap_or({});
    sfc::expect_eq(Str::from_utf8(s10), "0123456789");

    const auto s20 = r.peak(20).unwrap_or({});
    sfc::expect_eq(Str::from_utf8(s20), "0123456789");
  }

  // read
  {
    u8 tmp[4] = {};
    sfc::expect_eq(r.read(tmp).ok(), Option{4U});
    sfc::expect_eq(Str::from_utf8(tmp), "0123");

    sfc::expect_eq(r.read(tmp).ok(), Option{4U});
    sfc::expect_eq(Str::from_utf8(tmp), "4567");

    sfc::expect_eq(r.read(tmp).ok(), Option{2U});
    sfc::expect_eq(Str::from_utf8({tmp, 2}), "89");
  }
}

SFC_TEST(buf_write) {
  auto out = Vec<u8>{};
  auto w = BufWriter<Vec<u8>&>{out};

  sfc::expect_eq(w.write(Str{"0123"}.as_bytes()).ok(), Option{4U});
  sfc::expect_eq(out.len(), 0);

  sfc::expect_eq(w.flush().is_ok(), true);
  sfc::expect_eq(Str::from_utf8(out.as_slice()), "0123");

  sfc::expect_eq(w.write(Str{"4567"}.as_bytes()).ok(), Option{4U});
  sfc::expect_eq(w.flush().is_ok(), true);
  sfc::expect_eq(Str::from_utf8(out.as_slice()), "01234567");

  sfc::expect_eq(w.flush().is_ok(), true);
  sfc::expect_eq(Str::from_utf8(out.as_slice()), "01234567");
}

}  // namespace sfc::io::test
