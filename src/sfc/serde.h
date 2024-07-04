#pragma once

#include "sfc/collections/vecmap.h"

namespace sfc::serde {

class Node;

using List = Vec<Node>;
using Dict = collections::VecMap<String, Node>;

struct Null {
  void fmt(auto& f) const {
    f.pad("null");
  }
};

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

template <class T>
struct Serialize;

template <>
struct Serialize<bool> {
  const bool& _val;

 public:
  auto serialize(auto& se) const {
    return se.ser_bool(_val);
  }
};

template <num::Int T>
struct Serialize<T> {
  const T& _val;

 public:
  auto serialize(auto& se) const {
    return se.ser_int(static_cast<i64>(_val));
  }
};

template <num::Float T>
struct Serialize<T> {
  const T& _val;

 public:
  auto serialize(auto& se) const {
    return se.ser_flt(static_cast<f64>(_val));
  }
};

template <class T, usize N>
struct Serialize<T[N]> {
  const T (&_val)[N];

 public:
  auto serialize(auto& se) const {
    if constexpr (__is_constructible(Str, decltype(_val))) {
      return se.ser_str(Str{_val});
    } else {
      return se.ser_list(_val);
    }
  }
};

template <class T>
struct Deserialize;

template <>
struct Deserialize<bool> {
  static auto deserialize(auto& de) -> Option<bool> {
    return de.des_bool();
  }
};

template <num::Int T>
struct Deserialize<T> {
  static auto deserialize(auto& de) -> Option<T> {
    return de.template des_int<T>();
  }
};

template <num::Float T>
struct Deserialize<T> {
  static auto deserialize(auto& de) -> Option<T> {
    return de.template des_flt<T>();
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

  static auto ser_int(i64 val) -> Node {
    return Node{static_cast<i64>(val)};
  }

  static auto ser_flt(f64 val) -> Node {
    return Node{val};
  }

  static auto ser_str(Str val) -> Node {
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
    auto se = Serializer{};
    if constexpr (__is_class(T)) {
      return val.serialize(se);
    } else {
      return Serialize<T>{val}.serialize(se);
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
    } else {
      return Deserialize<T>::deserialize(*this);
    }
  }
};

auto serialize(const auto& val) -> Node {
  return Serializer::ser(val);
}

}  // namespace sfc::serde
