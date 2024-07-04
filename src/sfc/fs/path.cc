#include "path.h"

#include <sys/stat.h>

namespace sfc::fs {

using ffi::CString;

struct Components {
  Str _inn;

 public:
  auto as_path() const -> Path {
    return Path{_inn};
  }

  auto next_back() -> Str {
    auto s = Str{};
    while (_inn) {
      s = this->_next_back();
      if (s && !(s == Str{"."})) {
        break;
      }
    }
    return s;
  }

 private:
  auto _next_back() -> Str {
    if (!_inn) return {};

    const auto p = _inn.rfind('/');
    if (!p) {
      const auto s = _inn;
      _inn = {};
      return s;
    }

    const auto i = *p;
    if (i == 0 && _inn.len() == 1) {
      _inn = {};
      return "/";
    }

    const auto s = _inn[{i + 1, _}];
    _inn = _inn[{0, cmp::max(i, 1UL)}];
    return s;
  }
};

auto Path::to_c_string() -> ffi::CString {
  auto res = ffi::CString::from(_inn);
  return res;
}

auto Path::file_name() const -> Str {
  const auto s = Components{_inn}.next_back();
  if (s == "." || s == ".." || s == "/") {
    return {};
  }
  return s;
}

auto Path::extension() const -> Str {
  const auto s = this->file_name();
  if (s.is_empty()) {
    return {};
  }

  const auto pos = s.rfind('.').unwrap_or(s.len());
  const auto res = s[{pos + 1, _}];
  return res;
}

auto Path::file_stem() const -> Str {
  const auto s = this->file_name();
  if (s.is_empty()) {
    return {};
  }

  const auto pos = s.rfind('.').unwrap_or(s.len());
  const auto res = s[{0, pos}];
  return res;
}

auto Path::parent() const -> Path {
  auto imp = Components{_inn};
  imp.next_back();

  const auto res = imp.as_path();
  return res;
}

auto Path::is_absolute() const -> bool {
  return _inn.starts_with('/');
}

auto Path::is_relative() const -> bool {
  return !_inn.starts_with('/');
}

}  // namespace sfc::fs
