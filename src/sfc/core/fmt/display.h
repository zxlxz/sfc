#pragma once

#include "sfc/core/str.h"

namespace sfc::fmt {

struct Debug {
 public:
  static void fmt(bool val, auto& f) {
    if (val) {
      f.pad("true");
    } else {
      f.pad("false");
    }
  }

  static void fmt(char val, auto& f) {
    f.write_char(val);
  }

  static void fmt(const void* val, auto& f) {
    char buf[8 * sizeof(val)];
    const auto uval = reinterpret_cast<usize>(val);
    const auto type = f._options.type('p');
    const auto sval = num::Int{uval}.to_str(buf, type);
    f.pad_num(false, sval);
  }

  static void fmt(trait::uint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 16];
    const auto sval = num::Int{val}.to_str(buf, f._options._type);
    f.pad_num(false, sval);
  }

  static void fmt(trait::sint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 16];
    const auto uval = val >= 0 ? val : 0 - val;
    const auto sval = num::Int{uval}.to_str(buf, f._options._type);
    f.pad_num(val < 0, sval);
  }

  static void fmt(trait::flt_ auto val, auto& f) {
    static constexpr auto DEFAULT_PREC = sizeof(val) == 4 ? 4U : 6U;
    char buf[8 * sizeof(val) + 16];
    const auto prec = f._options.precision(DEFAULT_PREC);
    const auto uval = val >= 0 ? val : 0 - val;
    const auto sval = num::Flt{uval}.to_str(buf, prec, f._options._type);
    f.pad_num(val < 0, sval);
  }

  template <class T, usize N>
  static void fmt(const T (&val)[N], auto& f) {
    if constexpr (requires { Str{val}; }) {
      Str{val}.fmt(f);
    } else {
      Slice{val, N}.fmt(f);
    }
  }

  template <trait::enum_ E>
  static void fmt(E val, auto& f) {
    using I = __underlying_type(E);

    if constexpr (requires { enum_name(val); }) {
      const auto s = enum_name(val);
      return f.pad(s);
    }

    if constexpr (requires { E::_COUNT_; }) {
      static const auto& NAMES = []<u32... I>(trait::idxs_t<I...>) {
        static const Str names[] = {str::enum_name<static_cast<E>(I)>()...};
        return names;
      }(trait::idxs_seq_t<static_cast<u32>(E::_COUNT_)>{});
      return f.pad(NAMES[static_cast<u32>(val)]);
    }

    char buf[8];
    const auto sval = num::Int{static_cast<I>(val)}.to_str(buf);
    f.pad(sval);
  }
};

struct Display {
  static void fmt(const auto& s, auto& f) {
    if constexpr (requires { s.fmt(f); }) {
      s.fmt(f);
    } else {
      fmt::Debug::fmt(s, f);
    }
  }
};

}  // namespace sfc::fmt
