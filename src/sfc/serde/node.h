#pragma once

#include "sfc/collections.h"

namespace sfc::serde {

class Node;

struct Null {
  void fmt(auto& f) const {
    const auto s = Str{"null"};
    s.fmt(f);
  }
};

using List = Vec<Node>;
using Dict = VecMap<String, Node>;

class [[nodiscard]] Node {
  using Inn = Variant<Null, bool, i64, f64, String, List, Dict>;
  Inn _inn;

 public:
  Node();
  ~Node();
  Node(Node&&) noexcept;
  Node& operator=(Node&&) noexcept;

  explicit Node(bool val);
  explicit Node(i32 val);
  explicit Node(u32 val);
  explicit Node(i64 val);
  explicit Node(u64 val);
  explicit Node(f64 val);
  explicit Node(Str val);
  explicit Node(List val);
  explicit Node(Dict val);

 public:
  auto is_null() const -> bool;
  auto is_bool() const -> bool;
  auto is_int() const -> bool;
  auto is_flt() const -> bool;
  auto is_str() const -> bool;
  auto is_list() const -> bool;
  auto is_dict() const -> bool;

  auto as_null() const -> Null;
  auto as_bool() const -> bool;
  auto as_int() const -> i64;
  auto as_flt() const -> f64;
  auto as_str() const -> Str;
  auto as_list() const -> const List&;
  auto as_dict() const -> const Dict&;

  auto operator[](usize idx) const -> const Node&;
  void push(Node val);

  auto operator[](Str key) const -> const Node&;
  void insert(Str key, Node val);

  void fmt(auto& f) const {
    _inn.map([&](const auto& val) { f.write(val); });
  }

 public:
  static auto from_json(Str text) -> Option<Node>;
  
  auto to_json() const -> String;
};

}  // namespace sfc::serde
