#pragma once

namespace sfc::dyn {

template <class X>
auto as(const auto& dyn) -> const X& {
  return *static_cast<const X*>(dyn._impl);
}

template <class X>
auto as_mut(auto& dyn) -> X& {
  return *static_cast<X*>(dyn._impl);
}

}  // namespace sfc::dyn
