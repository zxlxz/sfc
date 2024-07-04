#pragma once

#include "deserialize.h"

namespace sfc::serde::des {

class Deserializer {
  const serde::Node& _node;

 public:
  using Node = serde::Node;

  Deserializer(const Node& node) : _node{node} {}

  auto des_null() const -> Null {
    return _node.as_null();
  }

  auto des_bool() const -> bool {
    return _node.as_bool();
  }

  auto des_int() const -> i64 {
    return _node.as_int();
  }

  auto des_flt() const -> f64 {
    if (_node.is_int()) {
      const auto int_val = _node.as_int();
      return static_cast<f64>(int_val);
    }
    return _node.as_flt();
  }

  auto des_str() const -> Str {
    if (_node.is_null()) {
      return "";
    }
    return _node.as_str();
  }

  auto des_list() const -> const List& {
    return _node.as_list();
  }

  auto des_dict() const -> const Dict& {
    return _node.as_dict();
  }

  template <class T>
  auto des() const -> T {
    return Deserialize<T>::deserialize(*this);
  }

  template <class T>
  auto des() const -> T
    requires(requires() { T::deserialize(*this); })
  {
    return T::deserialize(*this);
  }
};

template <class T>
[[nodiscard]] auto deserialize(const Node& node) -> T {
  auto imp = Deserializer{node};
  auto res = imp.des<T>();
  return res;
}

}  // namespace sfc::serde::des

namespace sfc::serde {
using des::deserialize;
}  // namespace sfc::serde
