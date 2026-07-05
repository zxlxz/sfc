#pragma once

#include "sfc/alloc.h"

namespace sfc::test {

using panic::SourceLoc;

struct Test {
  using func_t = void (*)();
  func_t _func;
  Str _path;
  SourceLoc _loc;

 public:
  auto mod() const noexcept -> Str;
  auto name() const noexcept -> Str;
  void run() const;
};

struct Module {
  Str _name;
  List<Test> _tests;

 public:
  explicit Module(Str name);
  ~Module();
  Module(Module&&) noexcept;
  Module& operator=(Module&&) noexcept;

  auto name() const noexcept -> Str;
  auto count() const noexcept -> usize;
  auto tests() const noexcept -> Slice<const Test>;
  void regist(Test test);
};

auto modules() noexcept -> Slice<const Module>;
auto regist_test(Test test) noexcept -> bool;

template <class T>
auto regist(SourceLoc loc = SourceLoc::current()) noexcept -> bool {
  const auto func = &T::test;
  const auto path = reflect::type_name<T>();
  return test::regist_test({func, path, loc});
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
