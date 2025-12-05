#include "sfc/core/fmt.h"

namespace sfc::fmt {

struct StyleParser {
  const char* _ptr;
  const char* _end;

 public:
  // [<fill>align][sign][#][0][width][.][precision][type]
  auto parse() -> Option<Style> {
    if (_ptr == _end) {
      return {};
    }

    auto res = Style{};

    // <fill>align
    const auto a0 = _ptr + 0 < _end ? _ptr[0] : 0;
    const auto a1 = _ptr + 1 < _end ? _ptr[1] : 0;
    if (a1 == '<' || a1 == '>' || a1 == '=' || a1 == '^') {
      res._fill = *_ptr++;
      res._align = *_ptr++;
    } else if (a0 == '<' || a0 == '>' || a0 == '=' || a0 == '^') {
      res._align = *_ptr++;
    }

    const auto sign = _ptr < _end ? _ptr[0] : 0;
    if (sign == '+' || sign == '-' || sign == ' ') {
      res._sign = *_ptr++;
    }

    if (_ptr < _end && _ptr[0] == '#') {
      res._prefix = *_ptr++;
    }

    if (res._fill == 0 && _ptr < _end && _ptr[0] == '0') {
      res._fill = *_ptr++;
    }

    res._width = this->extract_int();

    if (_ptr < _end && _ptr[0] == '.') {
      res._point = *_ptr++;
      res._precision = this->extract_int();
    }

    if (_ptr != _end) {
      res._type = *_ptr++;
    }
    return res;
  }

 private:
  auto extract_int() -> u8 {
    auto n = 0;
    for (; _ptr != _end; ++_ptr) {
      const auto c = *_ptr;
      if (c < '0' || c > '9') {
        break;
      }
      n = 10 * n + (c - '0');
    }
    return n < 255 ? static_cast<u8>(n) : static_cast<u8>(255);
  }
};

auto Style::from_str(str::Str s) noexcept -> Option<Style> {
  if (s[0] == ':') {
    s = s[{1, ops::$}];
  }

  auto imp = StyleParser{s._ptr, s._ptr + s._len};
  auto res = imp.parse();
  return res;
}

}  // namespace sfc::fmt
