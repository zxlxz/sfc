#include "sfc/env.h"

#include "sfc/test/test.h"

namespace sfc::env::test {

SFC_TEST(var) {
#ifdef _WIN32
  sfc::assert_eq(env::var("OS"), "Windows_NT");
#else
  sfc::assert_ne(env::var("HOME"), "");
#endif
}

SFC_TEST(set_var) {
  const auto key = Str{"SFC_TEST_ENV_VAR"};
  const auto val = Str{"test_value"};

  sfc::assert_eq(env::set_var(key, val), true);
  sfc::assert_eq(env::var(key), val);

  sfc::assert_eq(env::set_var(key, ""), true);
  sfc::assert_eq(env::var(key), "");

  sfc::assert_eq(env::remove_var(key), true);
  sfc::assert_eq(env::var(key), "");
}

SFC_TEST(current_dir) {
  const auto cwd = env::current_dir();
  sfc::assert_ne(cwd.as_str(), "");

  sfc::assert_eq(env::set_current_dir(*cwd), true);
  sfc::assert_eq(env::current_dir().as_str(), cwd.as_str());
}

SFC_TEST(home_dir) {
  const auto home = env::home_dir();
  sfc::assert_ne(home.as_str(), "");
}

SFC_TEST(current_exe) {
  const auto exe = env::current_exe();
  sfc::assert_ne(exe.as_str(), "");
  sfc::assert_eq(exe.as_str().contains("sfc-test"), true);
}

}  // namespace sfc::env::test
