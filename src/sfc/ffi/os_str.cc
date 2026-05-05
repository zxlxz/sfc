
#include "sfc/ffi/os_str.h"

namespace sfc::ffi {

auto CString::from(Str s) -> CString {
  if (s.is_empty()) {
    return {};
  }

  auto res = CString{};
  auto& buf = res._buf;
  buf.reserve(s.len() + 1);
  buf.extend_from_slice(s.as_bytes());
  buf.push(0);
  return res;
}

auto CString::ptr() const -> const char* {
  const auto p = _buf.as_ptr();
  return reinterpret_cast<const char*>(p);
}

auto CString::into_string() && -> String {
  auto buf = mem::move(_buf);
  if (!buf.is_empty()) {
    buf.pop();
  }
  return String::from_buf(mem::move(buf));
}

auto WString::from(Str s) -> WString {
  if (s.is_empty()) {
    return {};
  }

  auto res = WString{};
  res._buf.reserve(s.len());
  s.chars().for_each([&](char32_t c) {
    wchar_t buf[2] = {};
    const auto len = chr::wide_encode(c, buf);
    res._buf.extend_from_slice({buf, len});
  });
  res._buf.push(0);

  return res;
}

auto WString::ptr() const -> const wchar_t* {
  return _buf.as_ptr();
}

auto WString::buf() -> Buf& {
  return _buf;
}

// using this name, just make CString and WString have the same API
// even though WString::into_string() is not consuming self.
auto WString::into_string() const -> String {
  if (_buf.len() <= 1) {
    return {};
  }

  const auto p = _buf.as_ptr();
  const auto n = _buf.len() - 1;

  auto res = String::with_capacity(n);
  chr::WChars{{}, p, p + n}.for_each([&](wchar_t ch) { res.push(ch); });
  return res;
}

}  // namespace sfc::ffi
