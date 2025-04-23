#include "sfc/core/fmt/style.h"

namespace sfc::fmt {

namespace {

template <usize N>
static auto is_contains(char c, const char (&s)[N]) {
  for (auto u : s) {
    if (c == u) {
      return true;
    }
  }

  return false;
}

struct StyleParser {
  str::Str _buf;

 public:
  auto parse() -> Style {
    auto ret = Style{};

    if (is_contains(_buf[1], "=<^>")) {
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
  auto pop() -> char {
    const auto c = _buf[0];
    _buf._ptr += 1;
    _buf._len -= 1;
    return c;
  }

  template <usize N>
  auto pop_match(const char (&v)[N]) -> char {
    const auto c = _buf[0];
    if (is_contains(c, v)) {
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

}  // namespace

auto Style::from_str(str::Str s) -> Option<Style> {
  if (s.is_empty()) {
    return Option{Style{}};
  }

  const auto res = StyleParser{s}.parse();
  return Option{res};
}

}  // namespace sfc::fmt
