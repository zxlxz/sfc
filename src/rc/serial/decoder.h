#pragma once

#include "rc/serial/mod.h"

namespace rc::serial {

template <class T>
struct Decoder {
  const Node& _self;
  auto read() const -> T = delete;
};

template<>
struct Decoder<unit> {
  const Node& _self;
  auto read() const -> unit { return _self.as_unit(); }
};

template <>
struct Decoder<bool> {
  const Node& _self;
  auto read() const -> bool { return _self.as_bool(); }
};

template <>
struct Decoder<u8> {
  const Node& _self;
  auto read() const -> u8 { return _self.as_int<u8>(); }
};

template <>
struct Decoder<u16> {
  const Node& _self;
  auto read() const -> u16 { return _self.as_int<u16>(); }
};

template <>
struct Decoder<u32> {
  const Node& _self;
  auto read() const -> u32 { return _self.as_int<u32>(); }
};

template <>
struct Decoder<u64> {
  const Node& _self;
  auto read() const -> u64 { return _self.as_int<u64>(); }
};

template <>
struct Decoder<i8> {
  const Node& _self;
  auto read() const -> i8 { return _self.as_int<i8>(); }
};

template <>
struct Decoder<i16> {
  const Node& _self;
  auto read() const -> i16 { return _self.as_int<i16>(); }
};

template <>
struct Decoder<i32> {
  const Node& _self;
  auto read() const -> i32 { return _self.as_int<i32>(); }
};

template <>
struct Decoder<i64> {
  const Node& _self;
  auto read() const -> i64 { return _self.as_int<i64>(); }
};

template <>
struct Decoder<Str> {
  const Node& _self;
  auto read() const -> Str { return _self.as_str(); }
};

template <>
struct Decoder<String> {
  const Node& _self;
  auto read() const -> String { return String::from(_self.as_str()); }
};

template <class T>
struct Decoder<Vec<T>> {
  const Node& _self;

  auto read() const -> Vec<T> {
    Vec<T> res = Vec<T>::with_capacity(_self.len());
    for (auto& item : _self.as_list()) {
      res.push(Decoder<T>{item}.read());
    }
    return res;
  }
};

}  // namespace rc::serial
