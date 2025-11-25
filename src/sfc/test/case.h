#pragma once

#include "sfc/alloc.h"

namespace sfc::test {

struct Case {
  Str _str;
  void (*_fun)();
  panicking::Location _loc;

 public:
  auto suite() const -> Str;
  auto name() const -> Str;

  auto run(bool color = false) const -> bool;
};

class Suite {
  String _name{};
  Vec<Case> _cases{};

 public:
  explicit Suite(Str name);
  ~Suite();

  Suite(Suite&&) noexcept = default;
  Suite& operator=(Suite&&) noexcept = default;

  auto name() const -> Str;
  auto tests() const -> Slice<const Case>;

  auto push(Case unit) -> Case&;
  void run(Slice<const Str> pats, bool color = false);
};

class AllTest {
  Vec<Suite> _suites;

 public:
  static auto instance() -> AllTest&;
  auto suites() -> Slice<Suite>;
  auto regist(Case test_case) -> bool;

  void run(Slice<const Str> pats, bool color = false);
};

template <class T>
auto auto_regist(panicking::Location loc = {}) -> bool {
  const auto full_name = str::type_name<T>();
  const auto test_name = full_name[{0, full_name.len() - 4}];
  static const auto test_case = Case{test_name, &T::test, loc};
  static const auto regist_val = AllTest::instance().regist(test_case);
  return regist_val;
}

}  // namespace sfc::test

#define SFC_TEST(X)                                             \
  struct X##_UT_ {                                              \
    static const bool _UT_;                                     \
    static void test();                                         \
  };                                                            \
  const bool X##_UT_::_UT_ = sfc::test::auto_regist<X##_UT_>(); \
  void X##_UT_::test()
