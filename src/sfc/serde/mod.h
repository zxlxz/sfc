#pragma once

#include "sfc/core.h"

namespace sfc::serde {

template <class T>
struct Serialize {
  const T& _self;

 public:
  template <class S>
  auto serialize(S& ser) const {
    if constexpr (trait::same_<T, bool>) {
      return ser.serialize_bool(_self);
    } else if constexpr (trait::int_<T>) {
      return ser.serialize_int(_self);
    } else if constexpr (trait::flt_<T>) {
      return ser.serialize_flt(_self);
    } else if constexpr (trait::same_<T, Str>) {
      return ser.serialize_str(_self);
    } else if constexpr (trait::array_<T>) {
      if constexpr (requires { Str{_self}; }) {
        return ser.serialize_str(Str{_self});
      } else {
        return Slice{_self}.serialize(ser);
      }
    } else {
      return _self.serialize(ser);
    }
  }
};

template <class T>
struct Deserialize {
  template <class D>
  static auto deserialize(D& des) -> Result<T, typename D::Error> {
    if constexpr (trait::same_<T, bool>) {
      return des.deserialize_bool();
    } else if constexpr (trait::int_<T>) {
      return des.template deserialize_int<T>();
    } else if constexpr (trait::flt_<T>) {
      return des.template deserialize_flt<T>();
    } else if constexpr (trait::same_<T, Str>) {
      return des.deserialize_str();
    } else {
      return T::deserialize(des);
    }
  }
};

}  // namespace sfc::serde
