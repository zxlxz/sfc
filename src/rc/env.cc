#include "rc.inl"

#include "rc/env.h"
#include "rc/sys.h"

namespace rc::env {

pub auto var(Str key) -> String {
  // redirect to sys::env
  return sys::env::var(key);
}

pub auto home_dir() -> fs::PathBuf {
  // redirect to sys::env
  return fs::PathBuf{sys::env::home_dir()};
}

pub auto current_dir() -> fs::PathBuf {
  // redirect to sys::env
  return fs::PathBuf{sys::env::current_dir()};
}

pub auto current_exe() -> fs::PathBuf {
  // redirect to sys::env
  return fs::PathBuf{sys::env::current_exe()};
}

pub auto set_current_dir(fs::Path p) -> void {
  // redirect to sys::env
  return sys::env::set_current_dir(p._inner);
}


}  // namespace rc::env
