#include "sfc/io/mod.h"
#include "sfc/test/test.h"

namespace sfc::io::test {

SFC_TEST(read) {
  auto sb = Str{"0123456789"}.as_bytes();

  u8 tmp[4] = {};
  sfc::assert_eq(sb.read(tmp).ok(), Option{4UZ});
  sfc::assert_eq(Str::from_utf8(tmp), "0123");

  sfc::assert_eq(sb.read(tmp).ok(), Option{4UZ});
  sfc::assert_eq(Str::from_utf8(tmp), "4567");

  sfc::assert_eq(sb.read(tmp).ok(), Option{2UZ});
  sfc::assert_eq(Str::from_utf8({tmp, 2}), "89");
}

SFC_TEST(read_exact) {
  auto sb = Str{"0123456789"}.as_bytes();
  auto r = io::DynRead{sb};

  u8 tmp[4] = {};
  sfc::assert_eq(r.read_exact(tmp).is_ok(), true);
  sfc::assert_eq(Str::from_utf8(tmp), "0123");

  sfc::assert_eq(r.read_exact(tmp).is_ok(), true);
  sfc::assert_eq(Str::from_utf8(tmp), "4567");

  sfc::assert_eq(r.read_exact(tmp).is_err(), true);
  sfc::assert_eq(Str::from_utf8({tmp, 2}), "89");
}

SFC_TEST(read_to_end) {
  auto sb = Str{"0123456789"}.as_bytes();
  auto r = io::DynRead{sb};

  auto v = List<u8>{};
  sfc::assert_eq(r.read_to_end(v).is_ok(), true);
  sfc::assert_eq(Str::from_utf8(v.as_slice()), "0123456789");
}

SFC_TEST(read_to_string) {
  auto sb = Str{"0123456789"}.as_bytes();
  auto r = io::DynRead{sb};

  auto str = String{};
  sfc::assert_eq(r.read_to_string(str), usize{10});
  sfc::assert_eq(str, "0123456789");
}

SFC_TEST(write) {
  u8 buf[32];
  auto out = Slice{buf};

  // Test basic write
  const auto data = Str{"abcd"}.as_bytes();
  sfc::assert_eq(out.write(data).ok(), Option{4UZ});
  sfc::assert_eq(Str::from_utf8({buf, 4}), "abcd");

  // Test partial write (buffer full)
  {
    auto null_buf = Slice{buf, 0};
    sfc::assert_eq(null_buf.write(Str{"x"}.as_bytes()).ok(), Option{0UZ});
  }
}

SFC_TEST(write_all) {
  u8 buf[32];
  auto out = Slice{buf};
  auto w = io::DynWrite{out};

  // Test write_all writes complete data
  sfc::assert_eq(w.write_all(Str{"0123456789"}.as_bytes()).is_ok(), true);
  sfc::assert_eq(Str::from_utf8({buf, 10}), "0123456789");

  // Test write_all handles empty data
  sfc::assert_eq(w.write_all(Str{}.as_bytes()).is_ok(), true);
}

SFC_TEST(write_str) {
  u8 buf[32];
  auto out = Slice{buf};
  auto w = io::DynWrite{out};

  // Test write_str
  sfc::assert_eq(w.write_str("hello").is_ok(), true);
  sfc::assert_eq(Str::from_utf8({buf, 5}), "hello");

  // Test write_str with empty string
  sfc::assert_eq(w.write_str("").is_ok(), true);
}

}  // namespace sfc::io::test
