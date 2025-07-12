#include "sfc/core/fmt/fmter.h"

namespace sfc::fmt {

auto Style::from_str(str::Str s) -> Option<Style> {
  struct Stream {
    const char* _ptr;
    const char* _end;

   public:
    auto len() const {
      return _end - _ptr;
    }

    auto pop() -> char {
      return _ptr == _end ? char{0} : *_ptr++;
    }

    void unget() {
      --_ptr;
    }

    auto pop_match(str::Str s) -> char {
      if (_ptr == _end) {
        return 0;
      }

      if (auto c = *_ptr; s.as_chars().contains(c)) {
        ++_ptr;
        return c;
      }

      return 0;
    }

    auto extract_int() -> u8 {
      auto n = 0;

      for (; _ptr != _end; ++_ptr) {
        const auto c = *_ptr;
        if (c < '0' || c > '9') {
          break;
        }
        n = 10 * n + (c - '0');
      }

      return static_cast<u8>(n);
    }
  };

  if (s.is_empty()) {
    return {};
  }

  auto res = Style{};
  auto ss  = Stream{s._ptr, s._ptr + s._len};

  if (ss.len() >= 2) {
    res._fill  = ss.pop();
    res._align = ss.pop_match("=<^>");
  }

  if (res._align == 0) {
    ss.unget();
    res._align = ss.pop_match("=<^>");
  }

  res._sign   = ss.pop_match("+-");
  res._prefix = ss.pop_match("#");
  if (res._fill == 0) {
    res._fill = ss.pop_match("0");
  }
  res._width = ss.extract_int();
  res._point = ss.pop_match(".");
  if (res._point == '.') {
    res._precision = ss.extract_int();
  }
  res._type = ss.pop();

  return res;
}

}  // namespace sfc::fmt
