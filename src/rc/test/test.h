#pragma once

#include "rc/alloc.h"
#include "rc/core.h"

namespace rc::test {

struct TestFn {
  Str _name;
  void (*_func)();

  pub auto match(Slice<const Str> pats) const noexcept -> bool;
};

struct Tests {
  Vec<TestFn> _funcs;

  pub void insert_fn(TestFn test);
  pub void invoke_if(Slice<const Str> pats) const;
};

pub auto _global_tests() -> Tests&;

pub void run(Slice<const Str> pats);

template <class Impl>
struct AutoInstall {
  explicit AutoInstall() noexcept {
    auto& tests = _global_tests();

#if defined(__clang__)
    // rc::test::AutoInstall<Impl>::...
    const auto desc = Str{__PRETTY_FUNCTION__};
    const auto pos1 = desc.find(u8'<').unwrap_or(0);
    const auto pos2 = desc.find(u8'>').unwrap_or(desc.len());
    const auto mode = desc.slice(pos1 + 1, pos2);
#elif defined(_MSC_VER)
    // rc::test::AutoInstall<Impl>::...
    const auto desc = Str{__FUNCTION__};
    const auto pos1 = desc.find(u8'<').unwrap_or(0);
    const auto pos2 = desc.find(u8'>').unwrap_or(desc.len());
    const auto mode = desc.slice(pos1 + 1, pos2);
#endif
    tests.insert_fn({mode, &Impl::call});
  }
};

}  // namespace rc::test

#define rc_test(test_func)                              \
  struct test_func {                                      \
    static void call();                                   \
  };                                                      \
  static rc::test::AutoInstall<test_func> _##test_func; \
  void test_func::call()
