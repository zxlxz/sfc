#pragma once

#include "sfc/core/io.h"
#include "sfc/core/trait.h"

namespace sfc::convert {

template <class T>
struct AsRef {
  T _ref;

 public:
  static auto as_ref(const auto& self) -> T {
    return T{self};
  }
};

}  // namespace sfc::convert

namespace sfc {
using convert::AsRef;
}
