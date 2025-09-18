#pragma once

#include "sfc/collections/vecmap.h"

#include "sfc/serde/mod.h"
#include "sfc/serde/json.h"

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


}  // namespace sfc::serde
