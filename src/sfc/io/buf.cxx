#include "sfc/io/buf.h"

#include "sfc/test/test.h"

namespace sfc::io::test {

SFC_TEST(buf_read) {
  auto buf = Str{"0123456789"}.as_bytes();
  auto dyn_read = Dyn<Read>{buf};
  auto buf_read = BufReader<Dyn<Read>>::new_(dyn_read);

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
  auto buf = List<u8>::with_capacity(256);
  auto dyn_write = Dyn<Write>{buf};
  auto buf_write = BufWriter<Dyn<Write>>::new_(dyn_write);

  sfc::assert_eq(buf_write.write(Str{"0123"}.as_bytes()).ok(), Option{4UZ});
  sfc::assert_eq(buf.len(), 0U);

  sfc::assert_eq(buf_write.flush().is_ok(), true);
  sfc::assert_eq(Str::from_utf8(buf.as_slice()), "0123");

  sfc::assert_eq(buf_write.write(Str{"4567"}.as_bytes()).ok(), Option{4UZ});
  sfc::assert_eq(buf_write.flush().is_ok(), true);
  sfc::assert_eq(Str::from_utf8(buf.as_slice()), "01234567");

  sfc::assert_eq(buf_write.flush().is_ok(), true);
  sfc::assert_eq(Str::from_utf8(buf.as_slice()), "01234567");
}

}  // namespace sfc::io::test
