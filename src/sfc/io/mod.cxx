#include "sfc/io/mod.h"
#include "sfc/test/test.h"

namespace sfc::io::test {

struct RBuf : io::Read {
  Slice<const u8> _buf;

 public:
  RBuf(Str s) : _buf{s.as_bytes()} {}

  auto read(Slice<u8> buf) {
    return _buf.read(buf);
  }
};

struct WBuf : io::Write {
  Slice<u8> _buf;

 public:
  WBuf(Slice<u8> buf) : _buf{buf} {}

  auto write(Slice<const u8> buf) -> Result<usize> {
    if (_buf.is_empty()) {
      return Error::UnexpectedEof;
    }
    const auto len = cmp::min(_buf._len, buf._len);
    ptr::copy_nonoverlapping(buf._ptr, _buf._ptr, len);
    return len;
  }
};

SFC_TEST(read) {
  auto sb = RBuf{"0123456789"};

  u8 tmp[4] = {};
  sfc::expect_eq(sb.read(tmp).ok(), Option{4U});
  sfc::expect_eq(Str::from_utf8(tmp), "0123");

  sfc::expect_eq(sb.read(tmp).ok(), Option{4U});
  sfc::expect_eq(Str::from_utf8(tmp), "4567");

  sfc::expect_eq(sb.read(tmp).ok(), Option{2U});
  sfc::expect_eq(Str::from_utf8({tmp, 2}), "89");
}

SFC_TEST(read_exact) {
  auto sb = RBuf{"0123456789"};

  u8 tmp[4] = {};
  sfc::expect_true(sb.read_exact(tmp).is_ok());
  sfc::expect_eq(Str::from_utf8(tmp), "0123");

  sfc::expect_true(sb.read_exact(tmp).is_ok());
  sfc::expect_eq(Str::from_utf8(tmp), "4567");

  sfc::expect_true(sb.read_exact(tmp).is_err());
  sfc::expect_eq(Str::from_utf8({tmp, 2}), "89");
}

SFC_TEST(read_to_end) {
  auto sb = RBuf{"0123456789"};

  auto v = Vec<u8>{};
  sfc::expect_true(sb.read_to_end(v).is_ok());
  sfc::expect_eq(Str::from_utf8(v.as_slice()), "0123456789");
}

SFC_TEST(read_to_string) {
  auto sb = RBuf{"0123456789"};

  auto str = String{};
  sfc::expect_true(sb.read_to_string(str).is_ok());
  sfc::expect_eq(str, "0123456789");
}

SFC_TEST(write) {
  u8 buf[32];
  auto wbuf{WBuf{buf}};

  // Test basic write
  const auto data = Str{"abcd"}.as_bytes();
  sfc::expect_eq(wbuf.write(data).ok(), Option{4U});
  sfc::expect_eq(Str::from_utf8({buf, 4}), "abcd");

  // Test partial write (buffer full)
  auto wbuf2{WBuf{{buf, 0}}};
  sfc::expect_true(wbuf2.write(Str{"x"}.as_bytes()).is_err());
}

SFC_TEST(write_all) {
  u8 buf[32];
  auto wbuf{WBuf{buf}};

  // Test write_all writes complete data
  sfc::expect_true(wbuf.write_all(Str{"0123456789"}.as_bytes()).is_ok());
  sfc::expect_eq(Str::from_utf8({buf, 10}), "0123456789");

  // Test write_all handles empty data
  sfc::expect_true(wbuf.write_all(Str{}.as_bytes()).is_ok());
}

SFC_TEST(write_str) {
  u8 buf[32];
  auto wbuf{WBuf{buf}};

  // Test write_str
  sfc::expect_true(wbuf.write_str("hello").is_ok());
  sfc::expect_eq(Str::from_utf8({buf, 5}), "hello");

  // Test write_str with empty string
  sfc::expect_true(wbuf.write_str("").is_ok());
}

}  // namespace sfc::io::test
