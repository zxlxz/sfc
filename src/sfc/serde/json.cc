#include "json.h"
#include "sfc/fs/file.h"

namespace sfc::serde::json {

struct Json2Node {
  Str _str;

 public:
  auto parse() -> Option<Node> {
    auto res = this->extract_node();
    return res;
  }

 private:
  auto extract_keyword() -> Option<Node> {
    const auto k = this->read_keyword();
    if (k == "null") return Node{};
    if (k == "true") return Node{true};
    if (k == "false") return Node{false};
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
    auto list = List{};

    this->pop();  // pop '['

    // check end
    if (this->peak_ctrl() == ']') {
      this->pop();
      return Node{mem::move(list)};
    }

    for (;;) {
      if (auto node = this->extract_node()) {
        list.push(mem::move(node).unwrap());
      } else {
        return {};
      }

      const auto ch = this->read_ctrl();
      if (ch != ',') {
        if (ch == ']') break;
        return {};
      }
    }

    return Node{mem::move(list)};
  }

  auto extract_dict() -> Option<Node> {
    auto dict = Dict{};

    // pop '{'
    this->pop();

    // check end
    if (this->peak_ctrl() == '}') {
      this->pop();
      return Node{mem::move(dict)};
    }

    for (;;) {
      this->trim();
      const auto key = this->read_str();
      if (!key) {
        return {};
      }

      {
        auto ch = this->read_ctrl();
        if (ch != ':') {
          return {};
        }
      }

      if (auto node = this->extract_node()) {
        dict.insert(String::from(key), mem::move(node).unwrap_unchecked());
      } else {
        return {};
      }

      {
        const auto ch = this->read_ctrl();
        if (ch != ',') {
          if (ch == '}') break;
          return {};
        }
      }
    }

    return Node{mem::move(dict)};
  }

  auto extract_node() -> Option<Node> {
    const auto ch = this->peak_ctrl();

    switch (ch) {
      case '[':
        return this->extract_list();
      case '{':
        return this->extract_dict();
      case '"':
        return this->extract_str();
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
      case '9':
        return this->extract_num();
      case 't':
      case 'f':
      case 'n':
        return this->extract_keyword();
      default:
        return {};
    }
  }

 private:
  void pop() {
    _str._ptr += 1;
    _str._len -= 1;
  }

  void trim() {
    _str = _str.trim_start();
  }

  auto peak_ctrl() -> char {
    this->trim();
    const auto ch = _str[0];
    return ch;
  }

  auto read_ctrl() -> char {
    this->trim();
    const auto ch = _str[0];
    this->pop();
    return ch;
  }

  auto read_str() -> Str {
    if (_str[0] != '"') return {};

    this->pop();

    const auto p = _str.find('"').unwrap_or(_str._len);
    if (p == _str._len) {
      return {};
    }

    const auto v = _str.split_at(p);
    const auto a = v.template get<0>();
    const auto b = v.template get<1>();
    _str = b[{1U, _str._len}];
    return a;
  }

  auto read_num() -> Str {
    const auto f = [](char c) {  //
      return !(c == '.' || c == '+' || c == '-' || ('0' <= c && c <= '9'));
    };

    const auto p = _str.find(f).unwrap_or(_str._len);
    const auto v = _str.split_at(p);
    const auto a = v.template get<0>();
    const auto b = v.template get<1>();
    _str = b;
    return a;
  }

  auto read_keyword() -> Str {
    const auto f = [](char c) { return !(('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z')); };
    const auto p = _str.find(f).unwrap_or(_str._len);
    const auto v = _str.split_at(p);
    const auto a = v.template get<0>();
    const auto b = v.template get<1>();
    _str = b;
    return a;
  }
};

auto parse(Str s) -> Option<Node> {
  auto imp = Json2Node{s};
  return imp.parse();
}

}  // namespace sfc::serde::json
