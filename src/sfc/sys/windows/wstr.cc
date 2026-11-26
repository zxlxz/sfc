#include "sfc/sys/windows/wstr.h"

namespace sfc::sys::windows {

auto cvt_utf8_w16(Slice<const char> in, Slice<wchar_t> buf) -> Slice<const wchar_t> {
  if (buf._len == 0) {
    return {};
  }

  auto pos = 0U;
  auto write_slice = [&](auto s) {
    if (pos + s._len >= buf._len) return;
    for (auto ch : s) {
      buf._ptr[pos] = ch;
      pos += 1;
    }
    buf._ptr[pos] = 0;
  };

  chr::decode_utf8(Slice{(const u8*)in._ptr, in._len}).for_each([&](u32 ch) {
    const auto cp = chr::encode_utf16(ch);
    write_slice(Slice{cp.data, cp.len});
  });

  return {buf._ptr, pos};
}

auto cvt_w16_utf8(Slice<const wchar_t> in, Slice<char> buf) -> Slice<const char> {
  if (buf._len == 0) {
    return {};
  }

  auto pos = 0U;
  auto write_slice = [&](auto s) {
    if (pos + s._len >= buf._len) return;
    for (auto ch : s) {
      buf._ptr[pos] = ch;
      pos += 1;
    }
    buf._ptr[pos] = '\0';
  };

  chr::decode_utf16(Slice{(const u16*)in._ptr, in._len}).for_each([&](u32 ch) {
    const auto cp = chr::encode_utf8(ch);
    write_slice(Slice{cp.data, cp.len});
  });

  return {buf._ptr, pos};
}

}  // namespace sfc::sys::windows
