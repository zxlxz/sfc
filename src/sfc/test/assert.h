#pragma once

#include "sfc/io.h"

namespace sfc::test {

using panicking::Location;

struct AssertFailed {};

template <class T>
struct Param {
  Str      name;
  const T& value;

  void fmt(auto&& f) const {
    f.write_fmt("{} = {}", name, value);
  }
};

auto flt_eq_ulp(f64 a, f64 b, u32 ulp = 4) -> bool;

void assert_failed(Location loc, const auto& msg, const auto&... pars) {
  (void)(io::println("assert failed `{}`:", msg));
  (void)(io::println("> at {}[{}]", loc.file, loc.line));
  (void)(io::println("  {}", pars), ...);
  throw AssertFailed{};
}

void assert_true(const auto& val, Location loc = {}) {
  if (!bool(val)) {
    test::assert_failed(loc, "val==true", Param{"val", val});
  }
}

void assert_false(const auto& val, Location loc = {}) {
  if (bool(val)) {
    test::assert_failed(loc, "val==false", Param{"val", val});
  }
}

void assert_eq(const auto& a, const auto& b, Location loc = {}) {
  if (!(a == b)) {
    test::assert_failed(loc, "(lhs == rhs)", Param{"lhs", a}, Param{"rhs", b});
  }
}

void assert_ne(const auto& a, const auto& b, Location loc = {}) {
  if (!(a != b)) {
    test::assert_failed(loc, "(lhs != rhs)", Param{"lhs", a}, Param{"rhs", b});
  }
}

void assert_flt_eq(const auto& a, const auto& b, Location loc = {}) {
  if (!flt_eq_ulp(a, b)) {
    test::assert_failed(loc, "(lhs == rhs)", Param{"lhs", a}, Param{"rhs", b});
  }
}

void assert_flt_ne(const auto& a, const auto& b, Location loc = {}) {
  if (flt_eq_ulp(a, b)) {
    test::assert_failed(loc, "(lhs != rhs)", Param{"lhs", a}, Param{"rhs", b});
  }
}

}  // namespace sfc::test
