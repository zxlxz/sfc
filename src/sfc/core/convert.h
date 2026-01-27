#pragma once

#include "sfc/core/mod.h"

namespace sfc::convert {

template <class F, class T>
concept Into = requires(F self) { T{self}; };

template <class F, class T>
concept AsRef = requires(const F& self) { static_cast<T>(self); };

}  // namespace sfc::convert
