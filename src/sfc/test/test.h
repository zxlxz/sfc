#pragma once

#include "sfc/alloc.h"

namespace sfc::test {

using Loc = panic::SourceLoc;

enum class Status {
  None,
  Ready,
  Running,
  Passed,
  Failed,
};

struct Test {
  using Fun = void (*)();
  Str _type;
  Fun _func;
  Loc _loc;
  Status _stat = Status::None;
  Test* _next = nullptr;

 public:
  auto mod_name() const noexcept -> Str;
  auto func_name() const noexcept -> Str;
  auto status() const noexcept -> Status;
  auto location() const noexcept -> Loc;

 public:
  void set_status(Status stat);
  auto run() noexcept -> bool;
};

void regist_imp(Test& test);
void for_each(void* ctx, void (*f)(void* ctx, Test& test));

template <class T>
auto regist(SourceLoc loc = SourceLoc::current()) noexcept -> bool {
  static auto test = Test{reflect::type_name<T>(), &T::test, loc};
  regist_imp(test);
  return true;
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
