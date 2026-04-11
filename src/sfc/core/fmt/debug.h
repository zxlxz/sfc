#pragma once

#include "sfc/core/reflect.h"

namespace sfc::fmt {

struct Debug {
  static auto format_int(Slice<char> buf, auto val, char type = 0) -> Str;
  static auto format_ptr(Slice<char> buf, auto val, char type = 0) -> Str;
  static auto format_flt(Slice<char> buf, auto val, u32 precision = 6, char type = 0) -> Str;

  static void fmt(bool val, auto& f) {
    f.write_str(val ? Str{"true"} : Str{"false"});
  }

  static void fmt(char val, auto& f) {
    f.write_char(val);
  }

  static void fmt(const void* val, auto& f) {
    char buf[16];
    const auto s = format_ptr(buf, val, f._spec._type);
    f.write_str(s);
  }

  static void fmt(num::uint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 8];
    const auto s = format_int(buf, val + 0, f._spec._type);
    f.pad_num(false, s);
  }

  static void fmt(num::sint_ auto val, auto& f) {
    char buf[8 * sizeof(val) + 8];
    const auto s = format_int(buf, val < 0 ? -val : val, f._spec._type);
    f.pad_num(val < 0, s);
  }

  static void fmt(num::float_ auto val, auto& f) {
    static constexpr auto DEFAULT_PRECISION = sizeof(val) == 4 ? 4U : 6U;
    char buf[8 * sizeof(val) + 16];

    const auto p = f._spec.precision(DEFAULT_PRECISION);
    const auto s = format_flt(buf, val < 0 ? -val : val, p, f._spec._type);
    f.pad_num(val < 0, s);
  }

  static void fmt(enum_ auto val, auto& f) {
    const auto s = reflect::enum_name(val);
    if (!s.is_empty()) {
      f.pad(s);
    } else {
      f.write_fmt("{}({})", reflect::type_name<decltype(val)>(), static_cast<i64>(val));
    }
  }

  template <usize N>
  static void fmt(const char (&val)[N], auto& f) {
    f.pad(val);
  }

  template <class T, usize N>
  static void fmt(const T (&val)[N], auto& f) {
    auto imp = f.debug_list();
    for (auto& element : val) {
      imp.entry(element);
    }
  }
};

}  // namespace sfc::fmt
