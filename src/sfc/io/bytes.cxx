#include "sfc/io/bytes.h"

#include "sfc/test/test.h"

namespace sfc::io::test {

SFC_TEST(cast_bytes) {
  const u32 data[2] = {0x01020304U, 0x05060708U};
  const auto bytes = cast_bytes(Slice{data});
  sfc::assert_eq(bytes.len(), 2UZ * sizeof(u32));
  sfc::assert_eq(bytes[0], u8{0x04});
  sfc::assert_eq(bytes[4], u8{0x08});
}

SFC_TEST(cast_bytes_mut) {
  u16 data[2] = {0, 0};
  auto bytes = cast_bytes_mut(Slice{data});
  sfc::assert_eq(bytes.len(), 2UZ * sizeof(u16));
  bytes[0] = 0xAB;
  bytes[2] = 0xCD;
  sfc::assert_eq(data[0], u16{0x00AB});
  sfc::assert_eq(data[1], u16{0x00CD});
}

SFC_TEST(bytes_of) {
  const u64 val = 0x1122334455667788ULL;
  const auto bytes = bytes_of(val);
  sfc::assert_eq(bytes.len(), sizeof(u64));
  sfc::assert_eq(bytes[0], u8{0x88});
  sfc::assert_eq(bytes[7], u8{0x11});
}

SFC_TEST(bytes_of_mut) {
  u32 val = 0;
  auto bytes = bytes_of_mut(val);
  sfc::assert_eq(bytes.len(), sizeof(u32));
  bytes[0] = 0x78;
  bytes[3] = 0x12;
  sfc::assert_eq(val, u32{0x12000078U});
}

}  // namespace sfc::io::test
