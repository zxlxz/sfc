#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::unix {

static auto getenv(const char* key) -> ffi::CString {
  const auto val = ::getenv(key);
  return ffi::CString::from(val);
}

static auto setenv(const char* key, const char* val) -> bool {
  if (val == nullptr) {
    return ::unsetenv(key) == 0;
  }

  return ::setenv(key, val, 1) == 0;
}

static auto unsetenv(const char* key) -> bool {
  return ::unsetenv(key) == 0;
}

static auto home_dir() -> ffi::CString {
  const auto uid = getuid();
  const auto usr = getpwuid(uid);
  if (!usr || !usr->pw_dir) {
    return {};
  }
  return ffi::CString::from(usr->pw_dir);
}

static auto temp_dir() -> ffi::CString {
#ifdef __APPLE__
  char buf[PATH_MAX];
  if (::confstr(_CS_DARWIN_USER_TEMP_DIR, buf, sizeof(buf)) <= 0) {
    return {};
  }
  return ffi::CString::from(buf);
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
  return ffi::CString::from(buf);
}

static auto getcwd() -> ffi::CString {
  char buf[1024];
  const auto s = ::getcwd(buf, sizeof(buf));
  return ffi::CString::from(s);
}

static auto chdir(const char* path) -> bool {
  const auto ret = ::chdir(path);
  return ret == 0;
}

}  // namespace sfc::sys::unix
