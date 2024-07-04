#pragma once

#include "sfc/alloc.h"

namespace sfc::io {

struct Error {
  int _id;

 public:
  explicit Error(int id);

  static auto last_os_error() -> Error;

  auto to_str() const -> Str;

    void fmt(auto& f) const {
    const auto s = this->to_str();
    f.pad(s);
  }
};

}  // namespace sfc::io
