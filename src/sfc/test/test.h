#pragma once

#include "sfc/alloc.h"

namespace sfc::test {

struct SourceLoc {
  const char* file;
  int line;

  static auto current(const char* file = __builtin_FILE(), int line = __builtin_LINE()) -> SourceLoc {
    return {file, line};
  }
};

struct Test {
  void (*_fun)();
  Str _type;
  SourceLoc _loc;

 public:
  auto suite() const -> Str;
  auto name() const -> Str;

  auto match(Str pat) const -> bool;
  auto run(bool color = false) const -> bool;

 public:
  template <class T>
  static constexpr auto of(SourceLoc loc = SourceLoc::current()) -> Test {
    static constinit auto type_name = reflect::type_name<T>();
    return Test{&T::test, type_name[{0, type_name.len() - 4}], loc};
  }
};

struct Suite {
  Str _name{};
  Vec<Test> _tests{};

 public:
  auto name() const -> Str;
  auto tests() const -> Slice<const Test>;

  void push(Test test);
  void run(bool color = false) const;

  auto match(Str pat) const -> bool;
  auto filter(Slice<const Str> pats) const -> Suite;
};

auto regist(Test test) -> bool;
auto suites() -> Slice<const Suite>;
void run(Slice<const Suite> suites, bool color = false);

auto filter(Slice<const Str> pats) -> Vec<Suite>;

}  // namespace sfc::test

#define SFC_TEST(X)                                                             \
  struct X##_UT_ {                                                              \
    static const bool _UT_;                                                     \
    static void test();                                                         \
  };                                                                            \
  const bool X##_UT_::_UT_ = sfc::test::regist(sfc::test::Test::of<X##_UT_>()); \
  void X##_UT_::test()
