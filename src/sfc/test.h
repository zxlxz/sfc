#pragma once

#include "sfc/collections/vecmap.h"

namespace sfc::test {

template <typename T>
static auto type_name() -> Str {
#if defined(__clang__) || defined(__GNUC__)
  static constexpr auto S1 = sizeof("Str sfc::test::type_name() [T =");
  static constexpr auto S2 = sizeof("_UT_]");
  return Str{__PRETTY_FUNCTION__ + S1, sizeof(__PRETTY_FUNCTION__) - S1 - S2};
#elif defined(_MSC_VER)
  static constexpr auto S1 = sizeof("struct sfc::str::Str __cdecl sfc::test::type_name<struct");
  static constexpr auto S2 = sizeof("UT_<(void)>");
  return Str{__FUNCSIG__ + S1, sizeof(__FUNCSIG__) - S1 - S2};
#endif
}

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
  void run_tests(Slice<const Str> pats, bool color = false);
};

class App {
  Vec<Suite> _suites{};

 public:
  App();
  ~App();

  App(const App&) = delete;
  App& operator=(const App&) = delete;

  static auto instance() -> App&;

  void help();

  void list() const;

  void list_xml(Str path) const;

  void run(Str filters, Option<bool> color = {});

  auto operator[](Str name) -> Suite&;

  template <class T>
  auto regist(panicking::Location loc = {}) -> const void* {
    static const auto testcase = Case{type_name<T>(), &T::test, loc};
    auto& suite = (*this)[testcase.suite()];
    return &suite.push(testcase);
  }
};

}  // namespace sfc::test

#define SFC_TEST(X)                                                         \
  struct X##_UT_ {                                                          \
    static const void* _UT_;                                                \
    static void test();                                                     \
  };                                                                        \
  const void* X##_UT_::_UT_ = sfc::test::App::instance().regist<X##_UT_>(); \
  void X##_UT_::test()
