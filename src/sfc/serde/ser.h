#pragma once

#include "sfc/serde/node.h"

namespace sfc::serde {

template <class T>
class Serialize;

template <reflect::iReflect T>
class Serialize<T> : T {
 public:
  auto serialize(const auto& se) const {
    const auto info = reflect_struct(static_cast<const T&>(*this));
    auto res = se.ser_dict();
    info.fields().map([&](const auto& item) {
      auto node = se.ser(item.value);
      res.insert(item.name, mem::move(node));
    });
    return res;
  }
};

template <>
class Serialize<bool> {
  bool _val;

 public:
  auto serialize(const auto& se) const {
    return se.ser_bool(_val);
  }
};

template <>
class Serialize<const char*> {
  const char* _val;

 public:
  auto serialize(const auto& se) const {
    return se.ser_str(_val);
  }
};

template <trait::Int T>
class Serialize<T> {
  T _val;

 public:
  auto serialize(const auto& se) const {
    return se.ser_int(static_cast<i64>(_val));
  }
};

template <trait::Float T>
class Serialize<T> {
  T _val;

 public:
  auto serialize(const auto& se) const {
    auto ret = se.ser_flt(static_cast<f64>(_val));
    return ret;
  }
};

template <trait::Enum T>
class Serialize<T> {
  T _val;

 public:
  auto serialize(const auto& se) const {
    const auto s = reflect::enum_name(_val);
    return se.ser_str(s);
  }
};

template <class T, usize N>
class Serialize<T[N]> {
  T _val[N];

 public:
  auto serialize(const auto& se) const {
    auto res = se.ser_list();
    for (auto& e : _val) {
      auto node = se.ser(e);
      res.push(mem::move(node));
    }
    return res;
  }
};

template <>
class Serialize<Str> : Str {
 public:
  auto serialize(const auto& se) const {
    return se.ser_str(*this);
  }
};

template <>
class Serialize<String> : String {
 public:
  auto serialize(const auto& se) const {
    return se.ser_str(*this);
  }
};

template <class S>
class FmtAdapter {
  const S& _ser;
  Node& _node;

 public:
  FmtAdapter(const S& ser, Node& node) : _ser{ser}, _node{node} {}

  struct DebugList {
    const S& _ser;
    Node& _root;

    void entry(const auto& element) {
      auto node = _ser.ser(element);
      _root.push(mem::move(node));
    }

    void entries(auto iter) {
      for (; auto e = iter.next();) {
        this->entry(e.get_unchecked());
      }
    }
  };

  struct DebugMap {
    const S& _ser;
    Node& _root;

    void entry(Str name, const auto& element) {
      auto node = _ser.ser(element);
      _root.insert(name, mem::move(node));
    }

    void entries(auto iter) {
      iter.for_each([&](auto&& item) {  //
        this->entry(item.template get<0>(), item.template get<1>());
      });
    }
  };

  struct DebugStruct {
    const S& _ser;
    Node& _root;

    void field(Str name, const auto& element) {
      auto node = _ser.ser(element);
      _root.insert(name, mem::move(node));
    }
  };

  auto debug_tuple() -> DebugList {
    _node = _ser.ser_list();
    return DebugList{_ser, _node};
  }

  auto debug_list() -> DebugList {
    _node = _ser.ser_list();
    return DebugList{_ser, _node};
  }

  auto debug_set() -> DebugList {
    _node = _ser.ser_list();
    return DebugList{_ser, _node};
  }

  auto debug_map() -> DebugMap {
    _node = _ser.ser_dict();
    return DebugMap{_ser, _node};
  }

  auto debug_struct() -> DebugStruct {
    _node = _ser.ser_dict();
    return DebugStruct{_ser, _node};
  }

  void write(const auto& val) {
    _node = _ser.ser(val);
  }
};

template <class T>
class Serialize : T {
 public:
  template <class S>
  auto serialize(const S& se) const {
    auto res = Node{};
    auto imp = FmtAdapter<S>{se, res};
    T::fmt(imp);
    return res;
  }
};

struct Serializer {
  using Node = serde::Node;
  using List = serde::List;
  using Dict = serde::Dict;

  auto ser_null() const -> Node {
    return Node{};
  }

  auto ser_bool(bool val) const -> Node {
    return Node{val};
  }

  auto ser_int(i64 val) const -> Node {
    return Node{val};
  }

  auto ser_flt(f64 val) const -> Node {
    return Node{val};
  }

  auto ser_str(Str val) const -> Node {
    return Node{val};
  }

  auto ser_list() const -> Node {
    return Node{List{}};
  }

  auto ser_dict() const -> Node {
    return Node{Dict{}};
  }

  auto ser(const auto& val) const -> Node
    requires(requires() { val.serialize(*this); })
  {
    return val.serialize(*this);
  }

  template <class T>
  auto ser(const T& val) const -> Node {
    return ptr::cast<Serialize<T>>(&val)->serialize(*this);
  }
};

auto serialize(const auto& val) -> Node {
  auto imp = Serializer{};
  auto res = imp.ser(val);
  return res;
}

}  // namespace sfc::serde::ser
