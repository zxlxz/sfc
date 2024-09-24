#include "style.h"

namespace sfc::fmt {

struct StyleParser {
  str::Str _buf;

 public:
  auto parse() -> Style {
    auto ret = Style{};

    if (this->match(_buf[1], "=<^>")) {
      ret._fill = this->pop();
      ret._align = this->pop();
    } else {
      ret._align = this->pop_match("=<^>");
    }

    ret._sign = this->pop_match("+-");
    ret._prefix = this->pop_match("#");
    if (ret._fill == 0) {
      ret._fill = this->pop_match("0");
    }
    ret._width = this->extract_int();
    ret._point = this->pop_match(".");
    ret._precision = ret._point == '.' ? this->extract_int() : 0;
    ret._type = this->pop();

    return ret;
  }

 private:
  [[sfc_inline]] auto pop() -> char {
    const auto c = _buf[0];
    _buf._ptr += 1;
    _buf._len -= 1;
    return c;
  }

  template <usize N>
  auto match(char c, const char (&v)[N]) const -> bool {
    for (auto i = 0U; i < N - 1; ++i) {
      if (c == v[i]) return true;
    }
    return false;
  }

  template <usize N>
  auto pop_match(const char (&v)[N]) -> char {
    const auto c = _buf[0];
    if (this->match(c, v)) {
      this->pop();
      return c;
    }
    return 0;
  }

  auto extract_int() -> u8 {
    auto n = 0U;

    for (;;) {
      const auto c = _buf[0];
      if (c == 0 || c < '0' || c > '9') {
        break;
      }
      this->pop();
      n = 10 * n + static_cast<u32>(c - '0');
    }

    return static_cast<u8>(n);
  }
};

auto Style::from_str(str::Str s) -> Option<Style> {
  if (s.is_empty()) {
    return Style{};
  }

  auto res = StyleParser{s}.parse();
  return res;
}

}  // namespace sfc::fmt
