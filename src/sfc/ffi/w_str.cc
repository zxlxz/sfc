
#include "sfc/ffi/w_str.h"

namespace sfc::ffi {

static auto wide_codelen(wchar_t h) -> u32 {
  if constexpr (sizeof(wchar_t) == sizeof(char32_t)) {
    return 1;
  } else {
    if (h < 0xD800) {
      return 1;
    }
    if (h < 0xDBFF) {
      return 2;
    }
    return 0;
  }
}

struct WChars {
  const wchar_t* _ptr;
  const wchar_t* _end;

  auto next() -> Option<char32_t> {
    if (_ptr >= _end) {
      return {};
    }

    const auto n = wide_codelen(*_ptr);
    if (_ptr + n > _end) {
      _ptr = _end;
      return 0xFFFD;
    }

    switch (n) {
      case 1: {
        const auto a = *_ptr++;
        return a;
      }
      case 2: {
        const auto a = *_ptr++;
        const auto b = *_ptr++;
        return 0x10000 + ((a - 0xD800) << 10) + (b - 0xDC00);
      }
      default: {
        _ptr += 1;
        return 0xFFFD;
      }
    }
  }
};

auto WString::from(Str s) -> WString {
  if (s.is_empty()) {
    return {};
  }

  auto chars = s.chars();

  auto v = Vec<wchar_t>::with_capacity(s.len());
  while (auto x = chars.next()) {
    const auto ch = *x;
    if (ch <= 0xFFFF) {
      v.push(static_cast<wchar_t>(ch));
    } else {
      const auto t = ch - 0x10000;
      v.push(static_cast<wchar_t>(0xD800 + (t >> 10)));
      v.push(static_cast<wchar_t>(0xDC00 + (t & 0x3FF)));
    }
  }
  v.push(0);

  return WString::from_vec(mem::move(v));
}

auto WString::from_vec(Vec<wchar_t> v) -> WString {
  auto res = WString{};
  res._vec = mem::move(v);
  return res;
}

void WString::push(char32_t ch) {
  if (_vec.len() != 0) {
    _vec.set_len(_vec.len() - 1);
  }

  if (ch <= 0xFFFF) {
    _vec.push(static_cast<wchar_t>(ch));
  } else {
    const auto t = ch - 0x10000;
    _vec.push(static_cast<wchar_t>(0xD800 + (t >> 10)));
    _vec.push(static_cast<wchar_t>(0xDC00 + (t & 0x3FF)));
  }
}

auto WString::into_string() && -> String {
  auto chars = WChars{_vec.as_ptr(), _vec.as_ptr() + _vec.len()};

  auto res = String::with_capacity(_vec.len());
  while (auto ch = chars.next()) {
    res.push(*ch);
  }
  return res;
}

}  // namespace sfc::ffi
