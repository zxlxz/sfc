#include "rc.inl"

#include "rc/env.h"
#include "rc/log.h"
#include "rc/test.h"

namespace rc::test {

pub auto TestFn::match(Slice<const Str> pats) const noexcept -> bool {
  /* check */
  auto len = usize(0);
  auto res = true;
  for (auto pat : pats) {
    const auto flag = pat[0];
    const auto mask = pat.slice_from(1);

    if (flag == '+' && mask.len() > len) {
      if (_name.starts_with(mask)) {
        len = mask.len();
        res = true;
      }
    }
    if (flag == '-' && mask.len() > len) {
      if (_name.starts_with(mask)) {
        len = mask.len();
        res = false;
      }
    }
  }
  return res;
}

pub void Tests::insert_fn(TestFn test) {
  _funcs.push(test);
}

pub void Tests::invoke_if(Slice<const Str> pats) const {
  for (auto& func : _funcs) {
    if (!func.match(pats)) {
      continue;
    }
    try {
      log::user(u8"\x1b[32m[>>]\x1b[0m {} ...", func._name);
      func._func();

      log::user(u8"\x1b[32m[<<]\x1b[0m {} ok", func._name);
    } catch (...) {
      log::user(u8"\x1b[31m[<<]\x1b[0m {} failed", func._name);
    }
  }
}

pub auto _global_tests() -> Tests& {
  static Tests res;
  return res;
}

pub void run(Slice<const Str> pats) {
  auto& tests = _global_tests();
  tests.invoke_if(pats);
}

}  // namespace rc::test
