#include "node.h"

namespace sfc::serde {

Node::Node() : _inn{Null{}} {}

Node::~Node() = default;

Node::Node(Node&&) noexcept = default;

Node& Node::operator=(Node&&) noexcept = default;

Node::Node(bool val) : _inn{val} {}

Node::Node(i32 val) : _inn{static_cast<i64>(val)} {}

Node::Node(u32 val) : _inn{static_cast<i64>(val)} {}

Node::Node(i64 val) : _inn{static_cast<i64>(val)} {}

Node::Node(u64 val) : _inn{static_cast<i64>(val)} {}

Node::Node(f64 val) : _inn{val} {}

Node::Node(Str val) : _inn{String{val}} {}

Node::Node(cstr_t val) : _inn{String{val}} {}

Node::Node(List val) : _inn{static_cast<List&&>(val)} {}

Node::Node(Dict val) : _inn{static_cast<Dict&&>(val)} {}

auto Node::is_null() const -> bool {
  return _inn.is<Null>();
}

auto Node::is_bool() const -> bool {
  return _inn.is<bool>();
}

auto Node::is_int() const -> bool {
  return _inn.is<i64>();
}

auto Node::is_flt() const -> bool {
  return _inn.is<f64>();
}

auto Node::is_str() const -> bool {
  return _inn.is<String>();
}

auto Node::is_list() const -> bool {
  return _inn.is<List>();
}

auto Node::is_dict() const -> bool {
  return _inn.is<Dict>();
}

auto Node::as_null() const -> Null {
  return _inn.as<Null>();
}

auto Node::as_bool() const -> bool {
  return _inn.as<bool>();
}

auto Node::as_int() const -> i64 {
  return _inn.as<i64>();
}

auto Node::as_flt() const -> f64 {
  return _inn.as<f64>();
}

auto Node::as_str() const -> Str {
  return _inn.as<String>();
}

auto Node::as_list() const -> const List& {
  return _inn.as<List>();
}

auto Node::as_dict() const -> const Dict& {
  return _inn.as<Dict>();
}

auto Node::operator[](usize idx) const -> const Node& {
  auto& list = _inn.as<List>();
  return list[idx];
}

void Node::push(Node val) {
  if (!_inn.is<List>()) {
    if (_inn.is<Null>()) {
      _inn = Inn{List{}};
    }
  }
  _inn.as_mut<List>().push(mem::move(val));
}

auto Node::operator[](Str key) const -> const Node& {
  auto& dict = _inn.as<Dict>();
  return dict[key];
}

void Node::insert(Str key, Node val) {
  if (!_inn.is<Dict>()) {
    if (_inn.is<Null>()) {
      _inn = Inn{Dict{}};
    }
  }

  auto& dict = _inn.as_mut<Dict>();
  dict.insert(String{key}, static_cast<Node&&>(val));
}

}  // namespace sfc::serde
