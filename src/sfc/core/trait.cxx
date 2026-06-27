#include "sfc/core/trait.h"
#include "sfc/test.h"

namespace sfc::trait::test {

struct A {
  A() = default;
  ~A() = default;

  A(const A&) = default;
  A(A&&) = default;

  A& operator=(const A&) = default;
  A& operator=(A&&) = default;
};

struct B {
  B() = default;
  ~B() = default;

  B(const B&) = default;
  B& operator=(const B&) = default;
};

struct C {
  C() = default;
  ~C() {}

  C(C&&) = default;
  C& operator=(C&&) = default;
};

template <class T>
struct X {
  T _t;

  X() = default;

  ~X() requires(trait::tv_drop_<T>) = default;
  ~X() {}

  X(const X&) requires(trait::tv_copy_<T>) = default;
  X(X&&) noexcept requires(!trait::tv_copy_<T>) {}
};

SFC_TEST(trait_drop) {
  static_assert(trait::tv_drop_<A>);
  static_assert(trait::tv_drop_<X<A>>);

  static_assert(trait::tv_drop_<B>);
  static_assert(trait::tv_drop_<X<B>>);

  static_assert(!trait::tv_drop_<C>);
  static_assert(!trait::tv_drop_<X<C>>);
}

SFC_TEST(trait_copy) {
  static_assert(trait::tv_copy_<A>);
  static_assert(trait::tv_copy_<X<A>>);

  static_assert(trait::tv_copy_<B>);
  static_assert(trait::tv_copy_<X<B>>);

  static_assert(!trait::tv_copy_<C>);
  static_assert(!trait::tv_copy_<X<C>>);
}

}  // namespace sfc::trait::test
