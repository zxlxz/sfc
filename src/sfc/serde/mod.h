#pragma once

#include "sfc/alloc.h"

namespace sfc::serde {

struct Serialize {
  template <class T>
  static auto serialize(const T& self, auto& ser) {
    if constexpr (requires { self.serialize(ser); }) {
      return self.serialize(ser);
    } else if constexpr (trait::same_<T, bool>) {
      return ser.serialize_bool(self);
    } else if constexpr (trait::same_<T, char>) {
      return ser.serialize_char(self);
    } else if constexpr (trait::int_<T>) {
      return ser.serialize_int(self);
    } else if constexpr (trait::flt_<T>) {
      return ser.serialize_flt(self);
    } else if constexpr (requires { Str{self}; }) {
      return ser.serialize_str(self);
    } else if constexpr (requires { Slice{self}; }) {
      return Slice{self}.serialize(ser);
    } else {
      static_assert(false, "Serialize::serialize: not serializable");
    }
  }
};

struct Deserialize {
  template <class T>
  static auto deserialize(auto& des) {
    if constexpr (requires { T::deserialize(des); }) {
      return T::deserialize(des);
    } else if constexpr (trait::same_<T, bool>) {
      return des.deserialize_bool();
    } else if constexpr (trait::same_<T, char>) {
      return des.deserialize_char();
    } else if constexpr (trait::int_<T>) {
      return des.template deserialize_int<T>();
    } else if constexpr (trait::flt_<T>) {
      return des.template deserialize_flt<T>();
    } else if constexpr (requires { T{Str{}}; }) {
      return des.deserialize_str();
    } else {
      static_assert(false, "Deserialize::deserialize: not deserializable");
    }
  }
};

}  // namespace sfc::serde
