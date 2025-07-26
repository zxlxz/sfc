#include "json.h"

#include "sfc/fs/file.h"

namespace sfc::serde::json {

struct JsonStr {
  const char* _ptr;
  const char* _end;

 public:
  auto parse() -> Option<Node> {
    this->trim();
    if (_ptr == _end) {
      return {};
    }

    switch (*_ptr) {
      case '[': return this->extract_list();
      case '{': return this->extract_dict();
      case '"': return this->extract_str();
      case '+':
      case '-':
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': return this->extract_num();
      case 't': return this->extract_key('t');
      case 'f': return this->extract_key('f');
      case 'n': return this->extract_key('n');
      default:  return {};
    }
  }

 private:
  auto extract_key(char c) -> Option<Node> {
    const auto k = this->read_key();
    if (c == 'n' && k == "null") {
      return Node{};
    }
    if (c == 't' && k == "true") {
      return Node{true};
    }
    if (c == 'f' && k == "false") {
      return Node{false};
    }
    return {};
  }

  auto extract_str() -> Option<Node> {
    const auto s = this->read_str();
    if (!s) {
      return {};
    }
    return Node{s};
  }

  auto extract_num() -> Option<Node> {
    const auto s = this->read_num();
    if (!s) {
      return {};
    }

    if (s.contains('.')) {
      return s.parse<f64>().map([](auto val) { return Node{val}; });
    }

    return s.parse<i64>().map([](auto val) { return Node{val}; });
  }

  auto extract_list() -> Option<Node> {
    if (!this->pop('[')) {
      return {};
    }

    if (this->pop(']')) {
      return Node{List{}};
    }

    auto list = List{};
    for (;;) {
      if (auto val = this->parse()) {
        list.push(static_cast<Node&&>(*val));
      } else {
        return {};
      }

      if (this->pop(',')) {
        continue;
      }
      if (this->pop(']')) {
        break;
      }
      return {};
    }

    return Node{static_cast<List&&>(list)};
  }

  auto extract_dict() -> Option<Node> {
    if (!this->pop('{')) {
      return {};
    }

    if (this->pop('}')) {
      return Node{Dict{}};
    }

    auto dict = Dict{};
    for (;;) {
      this->trim();
      const auto key = this->read_str();
      if (!key) {
        return {};
      }

      if (!this->pop(':')) {
        return {};
      }

      if (auto val = this->parse()) {
        dict.insert(String::from(key), static_cast<Node&&>(*val));
      } else {
        return {};
      }

      if (this->pop(',')) {
        continue;
      }
      if (this->pop('}')) {
        break;
      }
      return {};
    }

    return Node{mem::move(dict)};
  }

 private:
  void trim() {
    for (; _ptr < _end; ++_ptr) {
      const auto c = *_ptr;
      if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
        break;
      }
    }
  }

  auto pop(char ch) -> bool {
    this->trim();
    if (_ptr == _end || *_ptr != ch) {
      return false;
    }
    ++_ptr;
    return true;
  }

  auto read_str() -> Str {
    if (*_ptr != '"') {
      return {};
    }

    const auto start = ++_ptr;
    for (; _ptr < _end; ++_ptr) {
      const auto c = *_ptr;
      if (c == '"') {
        break;
      }
    }
    if (_ptr >= _end) {
      return {};
    }

    const auto end = _ptr++;
    const auto len = static_cast<usize>(end - start);
    return Str{start, len};
  }

  auto read_num() -> Str {
    const auto start = _ptr;
    for (; _ptr < _end; ++_ptr) {
      const auto c = *_ptr;
      if (c >= '0' && c <= '9') {
        continue;
      } else if (c == '+' || c == '-' || c == '.') {
        continue;
      } else {
        break;
      }
    }
    return Str{start, static_cast<usize>(_ptr - start)};
  }

  auto read_key() -> Str {
    const auto start = _ptr;
    for (; _ptr < _end; ++_ptr) {
      const auto c = *_ptr | 32;
      if (c < 'a' || c > 'z') {
        break;
      }
    }
    return Str{start, static_cast<usize>(_ptr - start)};
  }
};

auto parse(Str s) -> Option<Node> {
  auto imp = JsonStr{s._ptr, s._ptr + s._len};
  auto res = imp.parse();
  return res;
}

}  // namespace sfc::serde::json
