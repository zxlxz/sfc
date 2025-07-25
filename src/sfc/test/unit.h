#pragma once

#include "sfc/collections/vecmap.h"

namespace sfc::test {

struct Unit {
  using Fun = void (*)();
  using Loc = panicking::Location;

  Str _str;
  Fun _fun;
  Loc _loc = {};

 public:
  template <typename T>
  static auto of(Loc loc) -> Unit {
#ifdef __clang__
    static constexpr auto S1 = sizeof("static Unit sfc::test::Unit::of(Loc) [T =");
    static constexpr auto S2 = sizeof("_UT_]");
#endif
    const auto name = Str{__PRETTY_FUNCTION__ + S1, sizeof(__PRETTY_FUNCTION__) - S1 - S2};
    return Unit{name, &T::test, loc};
  }

  auto invoke(bool color = false) const -> bool;

  auto suite() const -> Str;
  auto name() const -> Str;

  auto match(Slice<const Str> pats) const -> bool;
};

class Suite {
  String _name{};
  Vec<Unit> _unit{};

 public:
  explicit Suite(Str name) : _name{String::from(name)} {}

  ~Suite() = default;

  Suite(Suite&&) noexcept = default;

  Suite& operator=(Suite&&) noexcept = default;

  auto name() const -> Str {
    return _name;
  }

  auto tests() const -> Slice<const Unit> {
    return _unit.as_slice();
  }

  auto push(Unit unit) -> const Unit& {
    _unit.push(unit);
    return _unit.last();
  }

  auto match(Slice<const Str> pats) const -> bool;

  void invoke(Slice<const Str> pats, bool color = false);
};

class Tester {
  Vec<Suite> _suites{};

 public:
  static auto instance() -> Tester&;

  auto suites() const -> Slice<const Suite>;

  auto operator[](Str name) -> Suite&;

  void invoke(Slice<const Str> pats, bool color = false);

  template <class T>
  static auto regist(panicking::Location loc = {}) -> const Unit* {
    static const auto unit = Unit::of<T>(loc);
    auto& suite = instance()[unit.suite()];
    return &suite.push(unit);
  }
};

}  // namespace sfc::test

#define SFC_TEST(X)                                                            \
  struct X##_UT_ {                                                             \
    static const sfc::test::Unit* _UT_;                                        \
    static void test();                                                        \
  };                                                                           \
  const sfc::test::Unit* X##_UT_::_UT_ = sfc::test::Tester::regist<X##_UT_>(); \
  void X##_UT_::test()
