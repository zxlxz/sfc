#include "rc.inl"
#include "rc/fs.h"
#include "rc/io/read.h"
#include "rc/serial/mod.h"

namespace rc::serial {

struct JsonParser {
  const u8* _ptr;
  const u8* _end;

  explicit JsonParser(const Str& text)
      : _ptr{text.as_ptr()}, _end{text.as_ptr() + text.len()} {}

  static auto is_blank(u8 ch) noexcept -> bool {
    return ch == u8' ' || ch == u8'\t' || ch == u8'\r' || ch == u8'\n';
  }

  void skip_blank() {
    while (_ptr != _end && JsonParser::is_blank(*_ptr)) ++_ptr;
  }

  auto len() const noexcept -> usize { return usize(_end - _ptr); }

  auto as_str() const noexcept -> Str {
    const auto cnt = usize(_end - _ptr);
    return Str{_ptr, cnt};
  }

  auto top() noexcept -> u8 {
    this->skip_blank();
    if (_ptr == _end) return u8(0);
    return *_ptr;
  }

  auto pop() noexcept -> u8 {
    this->skip_blank();
    if (_ptr == _end) return u8(0);
    return *_ptr++;
  }

  auto pop_str(Str s) -> bool {
    const auto res = this->as_str().starts_with(s);
    if (res) {
      _ptr += s.len();
    }
    return res;
  }

  auto extract_chr(u8 c) noexcept -> u8 {
    this->skip_blank();

    if (_ptr == _end || *_ptr != c) return false;
    ++_ptr;
    return true;
  }

  auto extract_num() noexcept -> Str {
    const auto p = _ptr;
    for (; _ptr < _end; ++_ptr) {
      const auto c = *_ptr;
      if (this->is_blank(c)) break;
      if (c == u8',' || c == u8']' || c == u8'}') break;
    }
    return Str{p, usize(_ptr - p)};
  }

  auto extract_str() noexcept -> Str {
    const auto p = _ptr;
    _ptr++;
    for (; _ptr < _end; ++_ptr) {
      const auto c = *_ptr;
      if (c == u8'"') break;
    }
    return Str{p, usize(_ptr - p)};
  }

  auto parse_null() -> Node {
    const auto res = this->pop_str(u8"null");
    if (!res) throw Type::Null;
    return Node::from_null();
  }

  auto parse_bool(bool val) -> Node {
    const auto res = val ? this->pop_str(u8"true") : this->pop_str(u8"false");
    if (!res) throw Type::Bool;
    return Node::from_bool(val);
  }

  auto parse_num() -> Node {
    const auto s = this->extract_num();
    return Node::from_num(s);
  }

  auto parse_str() -> Node {
    const auto s = this->extract_str();
    return Node::from_str(s);
  }

  auto parse_list() -> Node {
    auto node = Node::from_list();
    auto& list = node.as_list_mut();

    // [
    ++_ptr;

    // ]
    if (this->top() == u8']') {
      ++_ptr;
      return node;
    }

    while (true) {
      list.push(this->parse());

      const auto next = this->pop();
      if (next == u8',') continue;
      if (next == u8']') break;
      throw Type::List;
    }

    return node;
  }

  auto parse_dict() -> Node {
    auto node = Node::from_dict();
    auto& dict = node.as_dict_mut();

    // {
    ++_ptr;

    // }
    if (this->top() == u8'}') {
      ++_ptr;
      return node;
    }

    while (true) {
      const auto key = this->extract_str();

      if (!this->extract_chr(u8':')) {
        throw Type::Dict;
      }
      dict.insert(key, this->parse());

      const auto next = this->pop();
      if (next == u8',') continue;
      if (next == u8'}') break;
      throw Type::Dict;
    }

    return node;
  }

  auto parse() -> Node {
    const auto c = this->top();

    switch (c) {
      case u8'n':  // null
        return this->parse_null();

      case u8't':
        return this->parse_bool(true);

      case u8'f':
        return this->parse_bool(false);

      case u8'"':
        return this->parse_str();

      case u8'+':
      case u8'-':
      case u8'0':
      case u8'1':
      case u8'2':
      case u8'3':
      case u8'4':
      case u8'5':
      case u8'6':
      case u8'7':
      case u8'8':
      case u8'9':
        return this->parse_num();

      case u8'[':
        return this->parse_list();

      case u8'{':
        return this->parse_dict();

      default:
        throw Type::Null;
    }
  }
};

struct JsonNode {
  const Node& _self;

  template <class I>
  auto fmt(fmt::Formatter<I>& formatter) const -> void {
    switch (_self.type()) {
      case Type::Null:
        formatter.write_str(u8"null");
        break;
      case Type::Bool:
        formatter.write(u8"{}", _self._data._bool);
        break;
      case Type::U64:
        formatter.write(u8"{}", _self._data._u64);
        break;
      case Type::I64:
        formatter.write(u8"{}", _self._data._i64);
        break;
      case Type::F64:
        formatter.write(u8"{}", _self._data._f64);
        break;
      case Type::Num:
        formatter.write(u8"{}", Str{_self._data._num, _self._len});
        break;
      case Type::Str:
        formatter.write(u8"\"{}\"", Str{_self._data._str, _self._len});
        break;
      case Type::List: {
        auto& list = _self.as_list();
        auto xfmt = formatter.debug_list();
        for (const auto& val : list) {
          xfmt.entry(JsonNode{val});
        }
        break;
      }
      case Type::Dict: {
        auto& list = _self.as_list();
        auto xfmt = formatter.debug_list();
        for (const auto& val : list) {
          xfmt.entry(JsonNode{val});
        }
        break;
      }
    }
  }
};

pub auto Node::from_json(Str s) -> Node {
  auto json_parser = JsonParser{s};
  return json_parser.parse();
}

pub auto Node::to_json() const -> String {
  const auto& json_node = reinterpret_cast<const JsonNode&>(*this);
  return string::format("{}", json_node);
}

}  // namespace rc::serial
