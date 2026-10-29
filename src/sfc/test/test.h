#pragma once

#include "sfc/alloc.h"

namespace sfc::test {

using Loc = panic::SourceLoc;

struct TestFn {
  using Fun = void (*)();
  Str _mod;
  Str _name;
  Loc _loc;
  Fun _func;

 public:
  auto run() const noexcept -> bool;
};

struct TestMod {
  Str _name;
  List<TestFn> _tests;

 public:
  explicit TestMod(Str name);
  ~TestMod();
  TestMod(TestMod&&) noexcept;
  TestMod& operator=(TestMod&&) noexcept;

  auto name() const noexcept -> Str;
  auto count() const noexcept -> usize;
  auto tests() const noexcept -> Slice<const TestFn>;
  void regist(TestFn test);
};

auto mods() noexcept -> Slice<const TestMod>;
auto regist_test(Str type, Loc loc, void (*fun)()) noexcept -> bool;

template <class T>
auto regist(SourceLoc loc = SourceLoc::current()) noexcept -> bool {
  const auto func = &T::test;
  const auto type = reflect::type_name<T>();
  return test::regist_test(type, loc, func);
}

}  // namespace sfc::test

#if defined(__INTELLISENSE__) || defined(__clang_analyzer__)
#define SFC_TEST(X) void _##X()
#else
#define SFC_TEST(X)                                  \
  struct _##X {                                      \
    static const bool _UT_;                          \
    static void test();                              \
  };                                                 \
  const bool _##X::_UT_ = sfc::test::regist<_##X>(); \
  void _##X::test()
#endif
