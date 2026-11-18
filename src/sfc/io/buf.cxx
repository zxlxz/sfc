#include "sfc/test.h"
#include "sfc/io/buf.h"

namespace sfc::io::test {

SFC_TEST(buf_read) {
  u8 buf[256];

  auto src = Str{"0123456789"}.as_bytes();
  auto buf_read = BufReader::new_(src, buf);

  // read
  {
    u8 tmp[4] = {};
    sfc::assert_eq(buf_read.read(tmp).ok(), Option{4UZ});
    sfc::assert_eq(Str::from_utf8(tmp), "0123");

    sfc::assert_eq(buf_read.read(tmp).ok(), Option{4UZ});
    sfc::assert_eq(Str::from_utf8(tmp), "4567");

    sfc::assert_eq(buf_read.read(tmp).ok(), Option{2UZ});
    sfc::assert_eq(Str::from_utf8({tmp, 2}), "89");
  }
}

SFC_TEST(buf_write) {
  u8 buf[256];

  auto dst = List<u8>::with_capacity(256);
  auto writer = BufWriter::new_(dst, buf);

  sfc::assert_eq(writer.write_str("0123").err(), None{});
  sfc::assert_eq(dst.len(), 0U);

  sfc::assert_eq(writer.flush().err(), None{});
  sfc::assert_eq(Str::from_utf8(dst.as_slice()), "0123");

  sfc::assert_eq(writer.write(Str{"4567"}.as_bytes()).ok(), Option{4UZ});
  sfc::assert_eq(writer.flush().err(), None{});
  sfc::assert_eq(Str::from_utf8(dst.as_slice()), "01234567");

  sfc::assert_eq(writer.flush().err(), None{});
  sfc::assert_eq(Str::from_utf8(dst.as_slice()), "01234567");
}

}  // namespace sfc::io::test
