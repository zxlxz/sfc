#pragma once

#include "sfc/alloc.h"

namespace sfc::collections {

template <class T>
class [[nodiscard]] VecSet {
  Vec<T> _items = {};

 public:
  VecSet() noexcept = default;

  ~VecSet() = default;

  VecSet(VecSet&&) noexcept = default;

  auto operator=(VecSet&&) noexcept -> VecSet& = default;

  auto len() const -> usize {
    return _items.len();
  }

  auto capacity() const -> usize {
    return _items.capacity();
  }

  auto is_empty() const -> bool {
    return _items.is_empty();
  }

  auto items() const -> Slice<const T> {
    return _items.as_slice();
  }

  auto items_mut() -> Slice<T> {
    return _items.as_mut_slice();
  }

  void reserve(usize additional) {
    _items.reserve(additional);
  }

  auto get(const auto& key) const -> Option<const T&> {
    return _items.iter().find([&](const T& t) { return key == t; });
  }

  auto get_mut(const auto& key) -> Option<T&> {
    return _items.iter_mut().find([&](const T& t) { return key == t; });
  }

  auto operator[](const auto& key) const -> const T& {
    auto opt = this->get(key);
    panicking::assert_fmt(opt, "VecSet::[]: key(=`{}`) not exists", key);
    return *opt;
  }

  auto operator[](const auto& key) -> T& {
    auto opt = this->get_mut(key);
    panicking::assert_fmt(opt, "VecSet::[]: key(=`{}`) not exists", key);
    return *opt;
  }

  void insert(T val) {
    auto pos = this->get_mut(val);
    if (pos) {
      *pos = static_cast<T&&>(val);
    } else {
      _items.push(static_cast<T&&>(val));
    }
  }

  auto remove(const auto& key) -> bool {
    const auto idx = _items.find_if([&](const T& t) { return key == t; });
    if (!idx) {
      return false;
    }
    _items.remove(*idx);
    return true;
  }

  void clear() {
    _items.clear();
  }

  auto contains(const auto& key) const -> bool {
    const auto val = this->get(key);
    return val;
  }

  void fmt(auto& f) const {
    _items.fmt(f);
  }
};

}  // namespace sfc::collections

namespace sfc {
using collections::VecSet;
}  // namespace sfc
