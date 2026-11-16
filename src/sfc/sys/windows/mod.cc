#include <corecrt_wstring.h>
#include "sfc/sys/windows/mod.h"

namespace sfc::sys::windows {

auto decode_wstr(Slice<const wchar_t> ws) -> chr::U16Chars {
  return chr::decode_utf16({(const u16*)ws._ptr, ws._len});
}

auto encode_wstr(Str s, Slice<wchar_t> buf) -> Slice<const wchar_t> {
  if (buf._len == 0) {
    return {};
  }

  if (s._len == 0) {
    buf._ptr[0] = 0;
    return {};
  }

  auto rem = buf;
  s.chars().for_each([&](char32_t ch) {
    u16 buf[2];
    const auto n = chr::encode_utf16(ch, buf)._len;
    (void)rem.write({(const wchar_t*)buf, n});
  });

  if (rem._len >= buf._len) {
    buf._ptr[0] = 0;
    return {};
  }

  const auto len = buf._len - rem._len;
  buf._ptr[len] = 0;
  return {buf._ptr, len};
}

auto encode_cstr(Slice<const wchar_t> ws, Slice<char> buf) -> cstr_t {
  if (ws._len == 0) {
    return nullptr;
  }

  auto cnt = usize{0};
  auto push_char = [&](char ch) {
    if (cnt < buf._len) {
      buf[cnt] = ch;
    }
    cnt += 1;
  };

  decode_wstr(ws).for_each([&](u32 ch) {
    u8 buf[4];
    const auto n = chr::encode_utf8(ch, buf)._len;
    for (auto i = 0U; i < n; ++i) {
      push_char(char(buf[i]));
    }
  });

  if (cnt >= buf._len) {
    return nullptr;
  }

  buf[cnt] = 0;
  return buf._ptr;
}

auto make_string(Slice<const wchar_t> ws) -> String {
  const auto u16_str = Slice{(const u16*)ws._ptr, ws._len};
  return String::from_utf16(u16_str);
}

}  // namespace sfc::sys::windows
