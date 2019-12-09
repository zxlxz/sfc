#pragma once

#include "rc/serial/mod.h"

namespace rc::serial {

template <class T>
struct Encoder {
  const T& _self;
  auto emit() const -> Node = delete;
};

template <>
struct Encoder<unit> {
  unit _self;
  auto emit() const -> Node { return Node::from_null(); }
};

template <>
struct Encoder<bool> {
  bool _self;
  auto emit() const -> Node { return Node::from_bool(_self); }
};

template <>
struct Encoder<u8> {
  u8 _self;
  auto emit() const -> Node { return Node::from_int(_self); }
};

template <>
struct Encoder<u16> {
  u16 _self;
  auto emit() const -> Node { return Node::from_int(_self); }
};

template <>
struct Encoder<u32> {
  u32 _self;
  auto emit() const -> Node { return Node::from_int(_self); }
};

template <>
struct Encoder<u64> {
  u64 _self;
  auto emit() const -> Node { return Node::from_int(_self); }
};

template <>
struct Encoder<i8> {
  i8 _self;
  auto emit() const -> Node { return Node::from_int(_self); }
};

template <>
struct Encoder<i16> {
  i16 _self;
  auto emit() const -> Node { return Node::from_int(_self); }
};

template <>
struct Encoder<i32> {
  i32 _self;
  auto emit() const -> Node { return Node::from_int(_self); }
};

template <>
struct Encoder<i64> {
  i64 _self;
  auto emit() const -> Node { return Node::from_int(_self); }
};

template <>
struct Encoder<f32> {
  f32 _self;
  auto emit() const -> Node { return Node::from_flt(_self); }
};

template <>
struct Encoder<f64> {
  f64 _self;
  auto emit() const -> Node { return Node::from_flt(_self); }
};

template <>
struct Encoder<Str> {
  Str _self;
  auto emit() const -> Node { return Node::from_str(_self); }
};

template <>
struct Encoder<String> {
  const String& _self;
  auto emit() const -> Node { return Node::from_str(_self.as_str()); }
};

template <class T>
struct Encoder<Slice<T>> {
  Slice<T> _self;

  auto emit() const -> Node {
    auto res = Node::from_list();
    auto& list = res.as_list_mut();
    for (auto& val : _self) {
      list.push(Encoder<T>{val}.emit());
    }
    return res;
  }
};

template <class T>
struct Encoder<Vec<T>> {
  const Vec<T>& _self;

  auto emit() const -> Node {
    auto res = Node::from_list();
    auto& list = res.as_list_mut();
    for (auto& val : _self) {
      list.push(Encoder<T>{val}.emit());
    }
    return res;
  }
};

}  // namespace rc::serial
