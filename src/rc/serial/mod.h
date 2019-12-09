#pragma once

#include "rc/alloc.h"

namespace rc::serial {

using string::String;

enum class Type: u32 {
  Null,
  Bool,
  U64,
  I64,
  F64,
  Num,
  Str,
  List,
  Dict,
};

struct List;
struct Dict;

struct Node {
  union Data {
    u64  _data;
    unit _null;
    bool _bool;
    u64 _u64;
    i64 _i64;
    f64 _f64;
    const u8* _num;
    const u8* _str;
    Node* _list;
    Tuple<Node,Node>* _dict;
  };

  Type _type;
  u32 _len;
  Data _data;

  pub explicit Node(Type type, u32 len, Data data)noexcept;
  pub Node(Node&& other) noexcept;
  pub ~Node();

  pub static auto from_null() noexcept -> Node;
  pub static auto from_bool(bool val) noexcept -> Node;
  pub static auto from_num(Str val) noexcept -> Node;
  pub static auto from_u64(u64 val) noexcept -> Node;
  pub static auto from_i64(i64 val) noexcept -> Node;
  pub static auto from_f64(f64 val) noexcept -> Node;
  pub static auto from_str(Str val) noexcept -> Node;
  pub static auto from_list() noexcept -> Node;
  pub static auto from_dict() noexcept -> Node;

  pub static auto from_json(Str s) -> Node;
  pub auto to_json() const -> String;

  pub auto len() const -> usize { return _len; }
  pub auto type() const -> Type { return _type; }

  pub auto as_unit() const -> unit;
  pub auto as_bool() const -> bool;
  pub auto as_u64() const -> u64;
  pub auto as_i64() const -> i64;
  pub auto as_f64() const -> f64;
  pub auto as_str() const -> Str;
  pub auto as_num() const -> Str;

  pub auto as_list() const -> const List&;
  pub auto as_list_mut() -> List&;

  pub auto as_dict() const -> const Dict&;
  pub auto as_dict_mut() -> Dict&;


  template<class T>
  static auto from_int(T val) -> Node {
    static_assert(rc::is_integeral<T>());
    if constexpr (rc::is_unsigned<T>()) {
      return Node::from_u64(val);
    }
    if constexpr (rc::is_signed<T>()) {
      return Node::from_i64(val);
    }
  }

  template <class T>
  auto as_int() const -> T {
    static_assert(rc::is_integeral<T>());
    if constexpr (rc::is_unsigned<T>()) {
      if (_type == Type::U64) return T(_data._u64);
    }
    if constexpr (rc::is_signed<T>()) {
      if (_type == Type::I64) return T(_data._i64);
    }
    return this->as_num().parse<T>().unwrap();
  }

  template <class T>
  static auto from_flt(T val) -> Node {
    static_assert(rc::is_floating_point<T>());
    return Node::from_f64(f64(val));
  }

  template <class T>
  auto as_flt() const -> T {
    static_assert(rc::is_floating_point<T>());
    if (_type == Type::F64) return T(_data._f64);
    return this->as_num().parse<T>().unwrap();
  }

  template<class Out>
  void fmt(fmt::Formatter<Out>& formatter) {
    const auto s = this->to_json();
    formatter.write_str(s);
  }
};

struct List: Node {
  using Cursor = slice::Cursor<const Node>;

  List() = delete;
  ~List() = delete;

  pub auto len() const -> usize;
  pub auto push(Node node) -> void;

  pub auto operator[](usize idx) const -> const Node&;
  pub auto operator[](usize idx) -> Node&;

  auto begin() const -> Cursor { return {_data._list}; }
  auto end() const -> Cursor { return {_data._list + _len}; }
};

struct Dict: Node {
  using Cursor = slice::Cursor<const Tuple<Node, Node>>;

  Dict() = delete;
  ~Dict() = delete;

  pub auto len() const -> usize;
  pub auto find(Str key) const -> Option<usize>;
  pub auto insert(Str key, Node val) -> void;

  pub auto operator[](Str key) const -> const Node&;
  pub auto operator[](Str key) -> Node&;

  auto begin() const -> Cursor { return Cursor{_data._dict}; }
  auto end() const -> Cursor { return Cursor{_data._dict + _len}; }
};

}  // namespace rc::serial

