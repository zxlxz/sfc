#pragma once

#include "sfc/core/reflect.h"

namespace sfc::fmt {

struct Debug {
  static void fmt(bool val, auto& f) {
    f.write_str(val ? Str{"true"} : Str{"false"});
  }

  static void fmt(char val, auto& f) {
    f.write_char(val);
  }

  static void fmt(const void* val, auto& f) {
    char buf[8 * sizeof(val)];
    const auto uval = reinterpret_cast<usize>(val);
    const auto type = f._spec.type('p');
    const auto sval = num::Int{uval}.to_str(buf, type);
    f.pad_num(false, sval);
  }

  static void fmt(trait::uint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 16];
    const auto sval = num::Int{val}.to_str(buf, f._spec._type);
    f.pad_num(false, sval);
  }

  static void fmt(trait::sint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 16];
    const auto uval = val >= 0 ? val : 0 - val;
    const auto sval = num::Int{uval}.to_str(buf, f._spec._type);
    f.pad_num(val < 0, sval);
  }

  static void fmt(trait::flt_ auto val, auto& f) {
    static constexpr auto DEFAULT_PREC = sizeof(val) == 4 ? 4U : 6U;
    char buf[8 * sizeof(val) + 16];
    const auto prec = f._spec.precision(DEFAULT_PREC);
    const auto uval = val >= 0 ? val : 0 - val;
    const auto sval = num::Flt{uval}.to_str(buf, prec, f._spec._type);
    f.pad_num(val < 0, sval);
  }

  static void fmt(trait::enum_ auto val, auto& f) {
    using I = __underlying_type(decltype(val));
    char buf[8];
    const auto sval = num::Int{static_cast<I>(val)}.to_str(buf);
    f.pad(sval);
  }
};

void Display::fmt(const auto& self, auto& f) {
  if constexpr (requires { self.fmt(f); }) {
    self.fmt(f);
  } else if constexpr (requires { reflect::to_str(self); }) {
    const auto s = reflect::to_str(self);
    f.write_str(s);
  } else if constexpr (requires { Str{self}; }) {
    Str{self}.fmt(f);
  } else if constexpr (requires { Slice{self}; }) {
    Slice{self}.fmt(f);
  } else {
    fmt::Debug::fmt(self, f);
  }
}

}  // namespace sfc::fmt
