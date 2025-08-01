#include "sfc/serde.h"

namespace sfc::serde {

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
    return Node{String::from(s)};
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

class JsonNode : public Node {
 public:
  void fmt(auto& f) const {
    this->map([&](const auto& val) { this->fmt_imp(val, f); });
  }

  static void fmt_imp(const Null& t, auto& f) {
    f.write_str("null");
  }

  static void fmt_imp(const bool& t, auto& f) {
    f.write_str(t ? Str{"true"} : Str{"false"});
  }

  static void fmt_imp(const i64& t, auto& f) {
    char buf[32];
    const auto s = num::int2str(buf, t);
    f.write_str(s);
  }

  static void fmt_imp(const f64& t, auto& f) {
    char buf[32];
    const auto s = num::flt2str(buf, t);
    f.write_str(s);
  }

  static void fmt_imp(const String& t, auto& f) {
    f.write_str("\"");
    f.write_str(t);
    f.write_str("\"");
  }

  static void fmt_imp(const List& t, auto& f) {
    const auto vals = t.as_slice();
    const auto cnt = t.len();

    f.write_str("[");
    for (auto i = 0U; i < cnt; ++i) {
      if (i != 0) {
        f.write_str(", ");
      }
      static_cast<const JsonNode&>(vals[i]).fmt(f);
    }
    f.write_str("]");
  }

  static void fmt_imp(const Dict& t, auto& f) {
    const auto keys = t.keys();
    const auto vals = t.vals();
    const auto cnt = t.len();

    f.write_str("{");
    for (auto i = 0U; i < cnt; ++i) {
      if (i != 0) {
        f.write_str(", ");
      }
      f.write_str("\"");
      f.write_str(keys[i]);
      f.write_str("\": ");
      static_cast<const JsonNode&>(vals[i]).fmt(f);
    }
    f.write_str("}");
  }
};

auto Node::from_json(Str text) -> Option<Node> {
  auto imp = JsonStr{text._ptr, text._ptr + text._len};
  return imp.parse();
}

auto Node::to_json() const -> String {
  auto buf = String{};
  auto out = fmt::Fmter{buf};
  static_cast<const JsonNode&>(*this).fmt(out);
  return buf;
}

}  // namespace sfc::serde
