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

auto regist(Case) -> bool;

template <class T>
static auto auto_regist(panicking::Location loc = {}) -> bool {
#if defined(__clang__) || defined(__GNUC__)
  static const auto S1 = sizeof("bool sfc::test::auto_regist(panicking::Location) [T = ");
  static const auto S2 = sizeof("_UT_]");
  static const auto SS = Str{__PRETTY_FUNCTION__ + S1, sizeof(__PRETTY_FUNCTION__) - S1 - S2};
#elif defined(_MSC_VER)
  static const auto S1 = sizeof("bool __cdecl sfc::test::regist<struct");
  static const auto S2 = sizeof("UT_<(void)>");
  static const auto SS = Str{__FUNCSIG__ + S1, sizeof(__FUNCSIG__) - S1 - S2};
#endif

  static const auto test_case = Case{SS, &T::test, loc};
  static const auto regist_val = test::regist(test_case);
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
