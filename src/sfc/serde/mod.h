#pragma once

#include "sfc/io.h"

namespace sfc::serde {

struct Serializer {
  auto serialize_null() = delete;
  auto serialize_bool(bool val) = delete;
  auto serialize_char(char val) = delete;
  auto serialize_int(trait::int_ auto val) = delete;
  auto serialize_flt(trait::flt_ auto val) = delete;
  auto serialize_str(Str val) = delete;
  auto serialize_bin(Slice<const u8> val) = delete;

  auto serialize_seq(Str name, auto&& iter) = delete;
  auto serialize_tuple(Str name, auto&& iter) = delete;
  auto serialize_struct(Str name, auto&& iter) = delete;
  auto serialize_map(Str name, auto&& iter) = delete;
};

struct Deserializer {
  auto deserialize_null(auto& visit) = delete;
  auto deserialize_bool(auto& visit) = delete;
  auto deserialize_char(auto& visit) = delete;
  auto deserialize_int(auto& visit) = delete;
  auto deserialize_flt(auto& visit) = delete;
  auto deserialize_str(auto& visit) = delete;
  auto deserialize_bin(auto& visit) = delete;

  auto deserialize_seq(auto& visit) = delete;
  auto deserialize_tuple(auto& visit) = delete;
  auto deserialize_struct(auto& visit) = delete;
  auto deserialize_map(auto& visit) = delete;
};

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
  static auto deserialize(T& self, auto& des) -> io::Result<> {
    if constexpr (requires { self.deserialize(des); }) {
      self.deserialize(des);
    } else if constexpr (trait::same_<T, bool>) {
      self = _TRY(des.deserialize_bool());
    } else if constexpr (trait::same_<T, char>) {
      self = _TRY(des.deserialize_char());
    } else if constexpr (trait::int_<T>) {
      self = _TRY(des.template deserialize_int<T>());
    } else if constexpr (trait::flt_<T>) {
      self = _TRY(des.template deserialize_flt<T>());
    } else if constexpr (requires { self = Str{}; }) {
      self = _TRY(des.deserialize_str());
    } else {
      static_assert(false, "Deserialize::deserialize: not deserializable");
    }
    return {};
  }
};

}  // namespace sfc::serde
