#pragma once

#include "sfc/core/reflect.h"

namespace sfc::fmt {

struct Debug {
  static void fmt(bool val, auto& f) {
    if (val) {
      f.write_str("true");
    } else {
      f.write_str("false");
    }
  }

  static void fmt(char val, auto& f) {
    f.write_char(val);
  }

  static void fmt(const void* val, auto& f) {
    if (val == nullptr) {
      f.write_str("nullptr");
    } else {
      char buf[16];
      const auto uval = reinterpret_cast<usize>(val);
      const auto type = f._spec.type('p');
      const auto sval = num::Int{uval}.to_str(buf, type);
      f.write_str(sval);
    }
  }

  static void fmt(trait::uint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 8];
    const auto sval = num::Int{val}.to_str(buf, f._spec._type);
    f.pad_num(false, sval);
  }

  static void fmt(trait::sint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 8];
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
    using E = decltype(val);
    using I = __underlying_type(E);
    if (const auto name = reflect::to_str(val); !name.is_empty()) {
      name.fmt(f);
    } else {
      char buf[16];
      f.write_str(reflect::type_name<E>());
      f.write_str("(");
      f.write_str(num::Int{static_cast<I>(val)}.to_str(buf));
      f.write_str(")");
    }
  }

  template <class T, usize N>
  static void fmt(const T (&val)[N], auto& f) {
    auto imp = f.debug_list();
    for (auto& element : val) {
      imp.entry(element);
    }
  }

  template <usize N>
  static void fmt(const char (&val)[N], auto& f) {
    f.pad(val);
  }
};

}  // namespace sfc::fmt
