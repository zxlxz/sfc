#include "sfc/env.h"

#include "sfc/test.h"

namespace sfc::env::test {

SFC_TEST(var) {
#ifdef _WIN32
  panicking::assert_eq(env::var("OS"), "Windows_NT");
#else
  panicking::assert_ne(env::var("HOME"), "");
#endif
}

SFC_TEST(set_var) {
  const auto key = Str{"SFC_TEST_ENV_VAR"};
  const auto val = Str{"test_value"};

  panicking::assert_true(env::set_var(key, val));
  panicking::assert_eq(env::var(key), val);

  panicking::assert_true(env::set_var(key, ""));
  panicking::assert_eq(env::var(key), "");

  panicking::assert_true(env::remove_var(key));
  panicking::assert_eq(env::var(key), "");
}

SFC_TEST(current_dir) {
  const auto cwd = env::current_dir();
  panicking::assert_ne(cwd.as_str(), "");

  panicking::assert_true(env::set_current_dir(cwd));
  panicking::assert_eq(env::current_dir().as_str(), cwd.as_str());
}

SFC_TEST(home_dir) {
  const auto home = env::home_dir();
  panicking::assert_ne(home.as_str(), "");
}

SFC_TEST(current_exe) {
  const auto exe = env::current_exe().as_str();
  panicking::assert_ne(exe, "");
  panicking::assert_true(exe.contains("sfc_test"));
}

}  // namespace sfc::env::test
