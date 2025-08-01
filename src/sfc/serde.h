#pragma once

#include "sfc/collections/vecmap.h"

namespace sfc::serde {

class Node;

struct Null {
  void fmt(auto& f) const {
    f.pad("null");
  }
};

using List = Vec<Node>;
using Dict = collections::VecMap<String, Node>;

class [[nodiscard]] Node {
  using Inn = Variant<Null, bool, i64, f64, String, List, Dict>;
  Inn _inn;

 public:
  Node() : _inn{Null{}} {}
  ~Node() = default;

  Node(Node&&) noexcept = default;
  Node& operator=(Node&&) noexcept = default;

  explicit Node(auto val) : _inn{mem::move(val)} {}

  template <class T>
  auto is() const -> bool {
    return _inn.is<T>();
  }

  template <class T>
  auto as() const -> const T& {
    return _inn.as<T>();
  }

  template <class T>
  auto as_mut() -> T& {
    return _inn.as_mut<T>();
  }

  void map(auto&& f) const {
    _inn.map(f);
  }

  void map_mut(auto&& f) {
    _inn.map_mut(f);
  }

  auto operator[](usize idx) const -> const Node& {
    return _inn.as<List>()[idx];
  }

  void push(Node val) {
    _inn.as_mut<List>().push(mem::move(val));
  }

  auto operator[](Str key) const -> const Node& {
    return _inn.as<Dict>()[key];
  }

  void insert(Str key, Node val) {
    _inn.as_mut<Dict>().insert(String::from(key), mem::move(val));
  }

 public:
  static auto from_json(Str text) -> Option<Node>;
  auto to_json() const -> String;

  void fmt(auto& f) const {
    _inn.fmt(f);
  }
};

struct Serializer {
  using Node = serde::Node;
  using List = serde::List;
  using Dict = serde::Dict;

 public:
  static auto new_list() -> Node {
    return Node{List{}};
  }

  static auto new_dict() -> Node {
    return Node{Dict{}};
  }

  static auto ser_null() -> Node {
    return Node{};
  }

  static auto ser_bool(bool val) -> Node {
    return Node{val};
  }

  static auto ser_int(const auto& val) -> Node {
    return Node{static_cast<i64>(val)};
  }

  static auto ser_flt(const auto& val) -> Node {
    return Node{static_cast<f64>(val)};
  }

  static auto ser_str(const auto& val) -> Node {
    return Node{String::from(val)};
  }

  static auto ser_list(const auto& iter) -> Node {
    auto list = List{};
    for (const auto& val : iter) {
      list.push(Serializer::ser(val));
    }
    return Node{mem::move(list)};
  }

  template <class T>
  static auto ser(const T& val) -> Node {
    auto serializer = Serializer{};
    if constexpr (__is_class(T)) {
      return val.serialize(serializer);
    }

    if constexpr (__is_same(T, bool)) {
      return Serializer::ser_bool(val);
    } else if constexpr (__is_same(double, T) || __is_same(float, T)) {
      return Serializer::ser_flt(val);
    } else if constexpr (__is_constructible(u64, T)) {
      return Serializer::ser_int(val);
    } else {
      return Node{string::format("{}", val)};
    }
  }
};

class Deserializer {
  const Node& _node;

 public:
  Deserializer(const Node& node) : _node{node} {}

  auto des_bool() const -> Option<bool> {
    if (_node.is<bool>()) {
      return _node.as<bool>();
    }
    return {};
  }

  template <class T>
  auto des_int() const -> Option<T> {
    if (!_node.is<i64>()) {
      return {};
    }
    const auto val = _node.as<i64>();
    return static_cast<T>(val);
  }

  template <class T>
  auto des_flt() const -> Option<T> {
    if (!_node.is<f64>()) {
      return {};
    }
    const auto val = _node.as<f64>();
    return static_cast<T>(val);
  }

  template <class T>
  auto des() const -> Option<T> {
    if constexpr (__is_class(T)) {
      return T::deserialize(*this);
    }

    if constexpr (__is_same(T, bool)) {
      return this->des_bool();
    } else if constexpr (__is_same(T, double) || __is_same(T, float)) {
      return this->des_flt<T>();
    } else if constexpr (__is_constructible(u64, T)) {
      return this->des_int<T>();
    } else {
      return T::deserialize(*this);
    }
  }
};

auto serialize(const auto& val) -> Node {
  return Serializer::ser(val);
}

}  // namespace sfc::serde
