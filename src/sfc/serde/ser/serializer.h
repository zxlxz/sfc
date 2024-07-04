#pragma once

#include "serialize.h"

namespace sfc::serde::ser {

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

[[nodiscard]] auto serialize(const auto& val) -> Node {
  auto imp = ser::Serializer{};
  auto res = imp.ser(val);
  return res;
}

}  // namespace sfc::serde::ser

namespace sfc::serde {
using ser::serialize;
}  // namespace sfc::serde
