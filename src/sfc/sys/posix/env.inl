#pragma once

#include "sfc/sys/posix/mod.inl"

namespace sfc::sys::posix {

static auto getenv(const char* key) -> ffi::CString {
  const auto c_val = Str::from_cstr(::getenv(key));
  return ffi::CString::from(c_val);
}

static auto setenv(const char* key, const char* val) -> bool {
  if (val == nullptr) {
    return ::unsetenv(key) != -1;
  }

  return ::setenv(key, val, 1) != -1;
}

static auto unsetenv(const char* key) -> bool {
  return ::unsetenv(key) != -1;
}

static auto home_dir() -> ffi::CString {
  const auto uid = getuid();
  const auto usr = getpwuid(uid);
  if (!usr || !usr->pw_dir) {
    return {};
  }

  const auto c_home = Str::from_cstr(usr->pw_dir);
  return ffi::CString::from(c_home);
}

static auto temp_dir() -> ffi::CString {
#ifdef __APPLE__
  char buf[PATH_MAX];
  if (::confstr(_CS_DARWIN_USER_TEMP_DIR, buf, sizeof(buf)) <= 0) {
    return {};
  }
  const auto c_temp = Str::from_cstr(buf);
  return ffi::CString::from(c_temp);
#else
  return ffi::CString::from("/tmp");
#endif
}

static auto current_exe() -> ffi::CString {
  char buf[1024];
#ifdef __APPLE__
  auto size = uint32_t{sizeof(buf)};
  if (_NSGetExecutablePath(buf, &size) != 0) {
    return {};
  }
#else
  if (::readlink("/proc/self/exe", buf, sizeof(buf)) == -1) {
    return {};
  }
#endif
  const auto c_exe = Str::from_cstr(buf);
  return ffi::CString::from(c_exe);
}

static auto getcwd() -> ffi::CString {
  char buf[1024];
  const auto s = ::getcwd(buf, sizeof(buf));
  const auto c_cwd = Str::from_cstr(s);
  return ffi::CString::from(c_cwd);
}

static auto chdir(const char* path) -> bool {
  return ::chdir(path) != -1;
}

}  // namespace sfc::sys::posix
