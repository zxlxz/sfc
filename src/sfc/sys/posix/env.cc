#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <pwd.h>
#include <sys/param.h>

#include "sfc/sys/posix/env.h"

namespace sfc::sys::posix {

static constexpr u32 kMaxKey = 256U;
static constexpr u32 kMaxVal = 4096U;

auto getenv(Str key) -> String {
  const auto c_key = FixedCStr<kMaxKey>::from_str(key);
  if (!c_key) {
    return {};
  }

  const auto c_val = ::getenv(c_key);
  if (c_val == nullptr) {
    return {};
  }

  return String::from(c_val);
}

auto setenv(Str key, Str val) -> bool {
  const auto c_key = FixedCStr<kMaxKey>::from_str(key);
  const auto c_val = FixedCStr<kMaxVal>::from_str(val);
  if (!c_key || val._len >= kMaxVal - 1) {
    return false;
  }

  const auto ret = ::setenv(c_key, c_val, 1);
  return ret != -1;
}

auto unsetenv(Str key) -> bool {
  const auto c_key = FixedCStr<kMaxKey>::from_str(key);
  if (!c_key) {
    return false;
  }

  const auto ret = ::unsetenv(c_key);
  return ret != -1;
}

auto home_dir() -> String {
  char buf[4096];
  struct passwd pwd{};
  struct passwd* usr = nullptr;
  const auto ret = ::getpwuid_r(getuid(), &pwd, buf, sizeof(buf), &usr);
  if (ret != 0 || usr == nullptr || usr->pw_dir == nullptr) {
    return {};
  }

  return String::from(usr->pw_dir);
}

auto chdir(Str path) -> bool {
  const auto c_path = FixedCStr<kMaxPath>::from_str(path);
  if (!c_path) {
    return false;
  }
  const auto ret = ::chdir(c_path);
  return ret != -1;
}

auto getcwd() -> String {
  char buf[kMaxPath];
  const auto p = ::getcwd(buf, sizeof(buf));
  if (p == nullptr) {
    return {};
  }

  return String::from(p);
}

}  // namespace sfc::sys::posix
