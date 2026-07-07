#include "sfc/io/buf.h"

#include "sfc/test/test.h"

namespace sfc::io::test {

SFC_TEST(buf_read) {
  const auto in = Str{"0123456789"};
  auto r = BufReader{in.as_bytes()};

  // peak
  {
    const auto s1 = r.peak(1).unwrap_or({});
    sfc::assert_eq(Str::from_utf8(s1), "0");

    const auto s10 = r.peak(10).unwrap_or({});
    sfc::assert_eq(Str::from_utf8(s10), "0123456789");

    const auto s20 = r.peak(20).unwrap_or({});
    sfc::assert_eq(Str::from_utf8(s20), "0123456789");
  }

  // read
  {
    u8 tmp[4] = {};
    sfc::assert_eq(r.read(tmp).ok(), Option{4UZ});
    sfc::assert_eq(Str::from_utf8(tmp), "0123");

    sfc::assert_eq(r.read(tmp).ok(), Option{4UZ});
    sfc::assert_eq(Str::from_utf8(tmp), "4567");

    sfc::assert_eq(r.read(tmp).ok(), Option{2UZ});
    sfc::assert_eq(Str::from_utf8({tmp, 2}), "89");
  }
}

SFC_TEST(buf_write) {
  auto w = BufWriter{List<u8>::with_capacity(256)};
  auto& out = w.inner();

  sfc::assert_eq(w.write(Str{"0123"}.as_bytes()).ok(), Option{4UZ});
  sfc::assert_eq(out.len(), 0U);

  sfc::assert_eq(w.flush().is_ok(), true);
  sfc::assert_eq(Str::from_utf8(out.as_slice()), "0123");

  sfc::assert_eq(w.write(Str{"4567"}.as_bytes()).ok(), Option{4UZ});
  sfc::assert_eq(w.flush().is_ok(), true);
  sfc::assert_eq(Str::from_utf8(out.as_slice()), "01234567");

  sfc::assert_eq(w.flush().is_ok(), true);
  sfc::assert_eq(Str::from_utf8(out.as_slice()), "01234567");
}

}  // namespace sfc::io::test
