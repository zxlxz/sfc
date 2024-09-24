#pragma once

#include "sfc/serde/node.h"

namespace sfc::serde {

template <class T>
struct Deserialize;

template <trait::Int T>
struct Deserialize<T> {
  static auto deserialize(const auto& de) -> T {
    const auto res = de.des_int();
    return static_cast<T>(res);
  }
};

template <trait::Float T>
struct Deserialize<T> {
  static auto deserialize(const auto& de) -> T {
    const auto res = de.des_flt();
    return static_cast<T>(res);
  }
};

template <trait::Enum T>
struct Deserialize<T> {
  static auto deserialize(const auto& de) -> T {
    const auto str = de.des_str();
    const auto res = reflect::enum_from_name<T>(str);
    assert_fmt(res, "serde::des<{}>: cannot parse `{}` to enum.", reflect::type_name<T>(), str);
    return *res;
  }
};

template <>
struct Deserialize<bool> {
  static auto deserialize(const auto& de) -> bool {
    const auto res = de.des_bool();
    return res;
  }
};

template <>
struct Deserialize<String> {
  static auto deserialize(const auto& de) -> String {
    const auto str = de.des_str();
    return String{str};
  }
};

template <class T>
struct Deserialize<Vec<T>> {
  template <class D>
  static auto deserialize(const D& de) -> Vec<T> {
    const auto& list = de.des_list();

    auto res = Vec<T>::with_capacity(list.len());
    for (const auto& e : list.as_slice()) {
      auto val = D{e}.template des<T>();
      res.push(static_cast<T&&>(val));
    }
    return res;
  }
};

template <trait::Class T>
struct Deserialize<T> {
  template <class D>
  static auto deserialize(const D& de) -> T {
    const auto& dict = de.des_dict();

    T res{};
    auto info = reflect_struct(res);
    info.fields().map([&](auto& field) {
      auto name = Str{field.name};
      auto node = dict.get(name);
      assert_fmt(node, "serde::des<{}>: `{}` not found.", reflect::type_name<T>(), name);
      Deserialize::des_field(D{*node}, field.value);
    });
    return res;
  }

 private:
  template <class D, class U>
  static void des_field(const D& de, const U& val) {
    const_cast<U&>(val) = de.template des<U>();
  }

  template <class D, class U, usize N>
  static void des_field(const D& de, const U (&val)[N]) {
    auto& list = de.des_list();
    assert_fmt(list.len() == N, "serde::des<{}[{}]>: len(=`{}`) not match", reflect::type_name<U>(),
               N, list.len());

    list.iter_mut().for_each_idx([&](auto j, auto& t) {  //
      const_cast<U&>(val[j]) = D{t}.template des<U>();   //
    });
  }
};

class Deserializer {
  const Node& _node;

 public:
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
auto deserialize(const Node& node) -> T {
  auto imp = Deserializer{node};
  auto res = imp.des<T>();
  return res;
}

}  // namespace sfc::serde
