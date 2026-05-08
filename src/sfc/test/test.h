#pragma once

#include "sfc/alloc.h"

namespace sfc::test {

using panic::SourceLoc;

struct Test {
  using func_t = void (*)();
  Str _mod;
  Str _name;
  func_t _func;
  SourceLoc _loc;

 public:
  static auto from(Str type, func_t func, SourceLoc loc) -> Test;
  auto mod() const noexcept -> Str;
  auto name() const noexcept -> Str;
  void run() const;

 public:
  template <class T>
  static constexpr auto of(SourceLoc loc = SourceLoc::current()) -> Test {
    return Test::from(reflect::type_name<T>(), &T::test, loc);
  }
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
auto regist(Test test) noexcept -> bool;

}  // namespace sfc::test

#if defined(__INTELLISENSE__) || defined(__clang_analyzer__)
#define SFC_TEST(X) void _##X()
#else
#define SFC_TEST(X)                                                       \
  struct _##X {                                                           \
    static const bool _UT_;                                               \
    static void test();                                                   \
  };                                                                      \
  const bool _##X::_UT_ = sfc::test::regist(sfc::test::Test::of<_##X>()); \
  void _##X::test()
#endif
