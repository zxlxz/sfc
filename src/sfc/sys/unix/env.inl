#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::env {

static auto getenv(const char* key) -> String {
  const auto val = ::getenv(key);
  return sys::make_string(val);
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

static auto home_dir() -> String {
  const auto uid = getuid();
  const auto usr = getpwuid(uid);
  if (!usr || !usr->pw_dir) {
    return {};
  }
  return sys::make_string(usr->pw_dir);
}

static auto temp_dir() -> String {
#ifdef __APPLE__
  char buf[PATH_MAX];
  if (::confstr(_CS_DARWIN_USER_TEMP_DIR, buf, sizeof(buf)) <= 0) {
    return {};
  }
  return sys::make_string(buf);
#else
  return sys::make_string("/tmp");
#endif
}

static auto current_exe() -> String {
  char buf[1024];
#ifdef __APPLE__
  auto size = uint32_t{BUF_SIZE};
  if (_NSGetExecutablePath(buf, &size) != 0) {
    return {};
  }
#else
  if (::readlink("/proc/self/exe", buf, sizeof(buf)) == -1) {
    return {};
  }
  return sys::make_string(buf);
#endif
}

static auto getcwd() -> String {
  char buf[1024];
  const auto s = ::getcwd(buf, sizeof(buf));
  return sys::make_string(s);
}

static auto chdir(const char* path) -> bool {
  const auto ret = ::chdir(path);
  return ret == 0;
}

}  // namespace sfc::sys::env
