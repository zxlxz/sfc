#include "sfc/core/fmt.h"

namespace sfc::fmt {

struct StyleStr {
  const char* _ptr;
  const char* _end;

 public:
  explicit StyleStr(str::Str s) noexcept : _ptr{s._ptr}, _end{s._ptr + s._len} {}

  // [<fill>align][sign][#][0][width][.][precision][type]
  auto parse() -> option::Option<Style> {
    if (_ptr == _end) {
      return {};
    }

    auto res = Style{};

    if (_end - _ptr >= 2) {
      const char c = _ptr[1];
      if (c == '=' || c == '<' || c == '^' || c == '>') {
        res._fill = *_ptr;
        res._align = c;
        _ptr += 2;
      }
    }

    if (res._align == 0) {
      res._align = this->pop_match('=', '<', '^', '>');
    }

    res._sign = this->pop_match('+', '-');
    res._prefix = this->pop_match('#');
    if (res._fill == 0) {
      res._fill = this->pop_match('0');
    }
    res._width = this->extract_int();
    res._point = this->pop_match('.');
    if (res._point == '.') {
      res._precision = this->extract_int();
    }
    res._type = this->pop();
    return res;
  }

 private:
  auto pop() -> char {
    if (_ptr == _end) {
      return 0;
    }
    return *_ptr++;
  }

  auto pop_match(auto... pats) -> char {
    if (_ptr == _end) {
      return 0;
    }

    const char c = *_ptr;
    if (((c != pats) && ...)) {
      return 0;
    }

    ++_ptr;
    return c;
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

auto Style::from_str(str::Str s) noexcept -> Option<Style> {
  if (s[0] == ':') {
    s = s[{1, ops::$}];
  }

  auto imp = StyleStr{s};
  auto res = imp.parse();
  return res;
}

}  // namespace sfc::fmt
