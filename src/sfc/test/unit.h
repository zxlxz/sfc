#pragma once

#include "sfc/collections/vecmap.h"

namespace sfc::test {

using panicking::Location;

class Unit {
  using Fun = void (*)();

  Location _loc;
  Str      _name;
  Fun      _func;

 public:
  Unit(Location loc, Str name, Fun func) : _loc{loc}, _name{name}, _func{func} {}

  auto suite() const -> Str;
  auto name() const -> Str;
  auto location() const -> Location;
  auto invoke() const -> bool;

  auto match(Slice<const Str> pats) const -> bool;
};

class Suite {
  String    _name;
  Vec<Unit> _tests;

 public:
  explicit Suite(Str name);
  ~Suite();
  Suite(Suite&&) noexcept;
  auto operator=(Suite&&) noexcept -> Suite&;

  auto name() const -> Str;
  auto tests() const -> Slice<const Unit>;

  void push(Unit unit);
  auto match(Slice<const Str> pats) const -> bool;
};

class TestManager {
  VecMap<Str, Suite> _suites;

 public:
  static auto instance() -> TestManager&;

  auto suites() const -> Slice<const Suite>;

  auto units(Slice<const Str> pats) const -> Vec<Unit>;

  template <class T>
  auto regist(Location loc = {}) -> usize {
    static auto name = reflect::type_name<T>();
    static auto unid = this->regist_imp({loc, name, &T::test});
    return unid;
  }

 private:
  TestManager();
  ~TestManager();
  TestManager(const TestManager&) = delete;

  auto regist_imp(Unit) -> usize;
};

}  // namespace sfc::test

#define SFC_TEST(X)                                                                      \
  struct X##_SFCT_ {                                                                     \
    static const usize UID;                                                              \
    static void        test();                                                           \
  };                                                                                     \
  const usize X##_SFCT_::UID = ::sfc::test::TestManager::instance().regist<X##_SFCT_>(); \
  void        X##_SFCT_::test()
