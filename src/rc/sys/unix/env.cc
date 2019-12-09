#include "rc/sys/unix.inl"

#include "rc/ffi.h"
#include "rc/io.h"

namespace rc::sys::unix::env {

static constexpr auto MAX_PATH_LEN = 1024;

auto var(Str key) -> String {
  const auto ckey = ffi::CString{key};
  const auto res = ::getenv(ckey);
  const auto val = Str::from_cstr(res);
  return String::from(val);
}

auto set_val(Str key, Str val) -> void {
  const auto ckey = ffi::CString{key};
  if (val.is_empty()) {
    ::unsetenv(ckey);
  } else {
    const auto cval = ffi::CString{val};
    ::setenv(ckey, cval, 1);
  }
}

auto home_dir() -> String {
  auto res = env::var("HOME");
  return res;
}

auto current_exe() -> String {
  char buf[MAX_PATH_LEN];

#if defined(__linux__)
  auto ret = ::readlink("/proc/self/exe", buf, sizeof(buf));
#elif defined(__APPLE__)
  u32 len = sizeof(buf);
  auto ret = ::_NSGetExecutablePath(buf, &len);
#else
  int ret = -1;
#endif

  if (ret == -1) {
    throw io::Error::last_os_error();
  }

  const auto res = Str::from_cstr(buf);
  return String::from(res);
}

auto current_dir() -> String {
  char buf[MAX_PATH_LEN];

  const auto ret = ::getcwd(buf, sizeof(buf));
  if (ret == nullptr) {
    throw io::Error::last_os_error();
  }
  const auto res = Str::from_cstr(buf);
  return String::from(res);
}

auto set_current_dir(Str path) -> void {
  const auto c_p = ffi::CString{path};
  const auto ret = ::chdir(c_p);
  if (ret == -1) {
    throw io::Error::last_os_error();
  }
}

}  // namespace rc::sys::unix::env
