#include "sfc/env.h"

#include "sfc/test/test.h"

namespace sfc::env::test {

SFC_TEST(var) {
#ifdef _WIN32
  sfc::expect_eq(env::var("OS"), "Windows_NT");
#else
  sfc::expect_ne(env::var("HOME"), "");
#endif
}

SFC_TEST(set_var) {
  const auto key = Str{"SFC_TEST_ENV_VAR"};
  const auto val = Str{"test_value"};

  sfc::expect_true(env::set_var(key, val));
  sfc::expect_eq(env::var(key), val);

  sfc::expect_true(env::set_var(key, ""));
  sfc::expect_eq(env::var(key), "");

  sfc::expect_true(env::remove_var(key));
  sfc::expect_eq(env::var(key), "");
}

SFC_TEST(current_dir) {
  const auto cwd = env::current_dir();
  sfc::expect_ne(cwd.as_str(), "");

  sfc::expect_true(env::set_current_dir(cwd));
  sfc::expect_eq(env::current_dir().as_str(), cwd.as_str());
}

SFC_TEST(home_dir) {
  const auto home = env::home_dir();
  sfc::expect_ne(home.as_str(), "");
}

SFC_TEST(current_exe) {
  const auto exe = env::current_exe().as_str();
  sfc::expect_ne(exe, "");
  sfc::expect_true(exe.contains("sfc_test"));
}

}  // namespace sfc::env::test
