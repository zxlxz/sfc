#include "sfc/io/mod.h"
#include "sfc/test/test.h"

namespace sfc::io::test {

struct RBuf : io::Read {
  Slice<const u8> _buf;

 public:
  explicit RBuf(Str s) : _buf{s.as_bytes()} {}

  auto read(Slice<u8> buf) {
    return _buf.read(buf);
  }
};

struct WBuf : io::Write {
  Slice<u8> _buf;

 public:
  explicit WBuf(Slice<u8> buf) : _buf{buf} {}

  auto write(Slice<const u8> buf) -> Result<usize> {
    if (_buf.is_empty()) {
      return Error::UnexpectedEof;
    }
    const auto len = cmp::min(_buf._len, buf._len);
    ptr::copy_nonoverlapping(buf._ptr, _buf._ptr, len);
    return {len};
  }

  auto flush() -> Result<> {
    return Ok{};
  }
};

SFC_TEST(read) {
  auto sb = RBuf{"0123456789"};

  u8 tmp[4] = {};
  sfc::assert_eq(sb.read(tmp).ok(), Option{4UZ});
  sfc::assert_eq(Str::from_utf8(tmp), "0123");

  sfc::assert_eq(sb.read(tmp).ok(), Option{4UZ});
  sfc::assert_eq(Str::from_utf8(tmp), "4567");

  sfc::assert_eq(sb.read(tmp).ok(), Option{2UZ});
  sfc::assert_eq(Str::from_utf8({tmp, 2}), "89");
}

SFC_TEST(read_exact) {
  auto sb = RBuf{"0123456789"};

  u8 tmp[4] = {};
  sfc::assert_eq(sb.read_exact(tmp).is_ok(), true);
  sfc::assert_eq(Str::from_utf8(tmp), "0123");

  sfc::assert_eq(sb.read_exact(tmp).is_ok(), true);
  sfc::assert_eq(Str::from_utf8(tmp), "4567");

  sfc::assert_eq(sb.read_exact(tmp).is_err(), true);
  sfc::assert_eq(Str::from_utf8({tmp, 2}), "89");
}

SFC_TEST(read_to_end) {
  auto sb = RBuf{"0123456789"};

  auto v = List<u8>{};
  sfc::assert_eq(sb.read_to_end(v).is_ok(), true);
  sfc::assert_eq(Str::from_utf8(v.as_slice()), "0123456789");
}

SFC_TEST(read_to_string) {
  auto sb = RBuf{"0123456789"};

  auto str = String{};
  sfc::assert_eq(sb.read_to_string(str).is_ok(), true);
  sfc::assert_eq(str, "0123456789");
}

SFC_TEST(write) {
  u8 buf[32];
  auto wbuf{WBuf{buf}};

  // Test basic write
  const auto data = Str{"abcd"}.as_bytes();
  sfc::assert_eq(wbuf.write(data).ok(), Option{4UZ});
  sfc::assert_eq(Str::from_utf8({buf, 4}), "abcd");

  // Test partial write (buffer full)
  auto wbuf2{WBuf{{buf, 0}}};
  sfc::assert_eq(wbuf2.write(Str{"x"}.as_bytes()).is_err(), true);
}

SFC_TEST(write_all) {
  u8 buf[32];
  auto wbuf{WBuf{buf}};

  // Test write_all writes complete data
  sfc::assert_eq(wbuf.write_all(Str{"0123456789"}.as_bytes()).is_ok(), true);
  sfc::assert_eq(Str::from_utf8({buf, 10}), "0123456789");

  // Test write_all handles empty data
  sfc::assert_eq(wbuf.write_all(Str{}.as_bytes()).is_ok(), true);
}

SFC_TEST(write_str) {
  u8 buf[32];
  auto wbuf{WBuf{buf}};

  // Test write_str
  sfc::assert_eq(wbuf.write_str("hello").is_ok(), true);
  sfc::assert_eq(Str::from_utf8({buf, 5}), "hello");

  // Test write_str with empty string
  sfc::assert_eq(wbuf.write_str("").is_ok(), true);
}

}  // namespace sfc::io::test
