#include "sfc/env.h"
#include "sfc/sys/env.h"

namespace sfc::env {

auto var(Str key) -> String {
  return sys::getenv(key);
}

auto set_var(Str key, Str val) -> bool {
  return sys::setenv(key, val);
}

auto remove_var(Str key) -> bool {
  return sys::unsetenv(key);
}

auto home_dir() -> String {
  return sys::home_dir();
}

auto current_dir() -> String {
  return sys::getcwd();
}

auto set_current_dir(Str path) -> bool {
  return sys::chdir(path);
}

}  // namespace sfc::env
