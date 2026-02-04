#pragma once

#include "sfc/sys/unix/mod.inl"

namespace sfc::sys::env {

template <size_t BUF_SIZE>
static auto getenv(const char* key, char (&buf)[BUF_SIZE]) -> const char* {
  const auto val = ::getenv(key);
  return to_utf8(val, buf);
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

template <size_t BUF_SIZE>
static auto home_dir(char (&buf)[BUF_SIZE]) -> const char* {
  const auto uid = getuid();
  const auto usr = getpwuid(uid);
  if (!usr) {
    return nullptr;
  }
  return to_utf8(usr->pw_dir, buf);
}

template <size_t BUF_SIZE>
static auto temp_dir(char (&buf)[BUF_SIZE]) -> const char* {
#ifdef __APPLE__
  static const auto len = ::confstr(_CS_DARWIN_USER_TEMP_DIR, buf, BUF_SIZE);
  if (len <= 0 || len >= BUF_SIZE) {
    return nullptr;
  }
  return buf;
#else
  return to_utf8("/tmp", buf);
#endif
}

template <size_t BUF_SIZE>
static auto current_exe(char (&buf)[BUF_SIZE]) -> const char* {
#ifdef __APPLE__
  auto size = uint32_t{BUF_SIZE};
  if (_NSGetExecutablePath(buf, &size) != 0) {
    return nullptr;
  }
#else
  const auto len = ::readlink("/proc/self/exe", buf, BUF_SIZE);
  if (len <= 0 || static_cast<size_t>(len) >= BUF_SIZE) {
    return nullptr;
  }
  buf[len] = '\0';
#endif
  return buf;
}

template <size_t BUF_SIZE>
static auto getcwd(char (&buf)[BUF_SIZE]) -> const char* {
  return ::getcwd(buf, BUF_SIZE);
}

static auto chdir(const char* path) -> bool {
  return ::chdir(path) == 0;
}

}  // namespace sfc::sys::env
