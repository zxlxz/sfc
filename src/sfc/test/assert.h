#pragma once

#include "sfc/io.h"
#include "cmp.h"

namespace sfc::test {

using panicking::Location;

struct AssertFailed {};

template <class T>
struct Param {
  Str name;
  const T& value;

  void fmt(auto& f) const {
    f.write_fmt("{} = {}", name, value);
  }
};

void assert_failed(Location loc, const auto& msg, const auto&... pars) {
  io::println("assert failed `{}`:", msg);
  io::println("> at {}[{}]", loc.file, loc.line);
  (void)(io::println("  {}", pars), ...);
  throw AssertFailed{};
}

template <class T>
void assert_true(const T& val, Location loc = {}) {
  if (static_cast<bool>(val)) return;
  assert_failed(loc, "val==true", Param<T>{"val", val});
}

template <class T>
void assert_false(const T& val, Location loc = {}) {
  if (!static_cast<bool>(val)) return;
  assert_failed(loc, "val==false", Param<T>{"val", val});
}

template <class A, class B>
void assert_eq(const A& a, const B& b, Location loc = {}) {
  if (a == b) return;
  assert_failed(loc, "(lhs == rhs)", Param<const A>{"lhs", a}, Param<const B>{"rhs", b});
}

template <class A, class B>
void assert_ne(const A& a, const B& b, Location loc = {}) {
  if (a != b) return;
  assert_failed(loc, "(lhs != rhs)", Param<A>{"lhs", a}, Param<B>{"rhs", b});
}

template <class A, class B>
void assert_flt_eq(const A& a, const B& b, Location loc = {}) {
  if (test::flt_eq(a, b)) return;
  assert_failed(loc, "(lhs == rhs)", Param<A>{"lhs", a}, Param<B>{"rhs", b});
}

template <class T>
void assert_flt_ne(const T& a, const T& b, Location loc = {}) {
  if (!test::flt_eq(a, b)) return;
  assert_failed(loc, "(lhs != rhs)", Param<T>{"lhs", a}, Param<T>{"rhs", b});
}

}  // namespace sfc::test

#define ASSERT_ANY_THROW(...)
