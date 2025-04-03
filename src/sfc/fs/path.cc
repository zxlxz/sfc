#include "path.h"

#include "meta.h"

namespace sfc::fs {

namespace {
struct Components {
  Str _inn;

 public:
  auto as_str() const -> Str {
    return _inn;
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
    if (!_inn)
      return {};

    const auto p = _inn.rfind('/');
    if (!p) {
      const auto s = _inn;
      _inn         = {};
      return s;
    }

    const auto i = *p;
    if (i == 0 && _inn.len() == 1) {
      _inn = {};
      return "/";
    }

    const auto s = _inn[{i + 1, _}];
    _inn         = _inn[{0, cmp::max(i, 1UL)}];
    return s;
  }
};
}  // namespace

Path::Path() = default;

Path::~Path() = default;

Path::Path(Path&&) noexcept = default;

Path& Path::operator=(Path&&) noexcept = default;

auto Path::from(Str s) -> Path {
  auto res = Path{};
  res._inn = String::from(s);
  return res;
}

auto Path::clone() const -> Path {
  return Path::from(_inn.as_str());
}

auto Path::as_str() const -> Str {
  return _inn.as_str();
}

auto Path::c_str() const -> cstr_t {
  return _inn.c_str();
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
  auto cmpts = Components{_inn};
  cmpts.next_back();

  return Path::from(cmpts.as_str());
}

auto Path::is_absolute() const -> bool {
  return _inn.starts_with('/');
}

auto Path::is_relative() const -> bool {
  return !_inn.starts_with('/');
}

void Path::push(Str path) {
  if (!_inn.ends_with('/')) {
    _inn.push('/');
  }
  if (path.starts_with('/')) {
    _inn.clear();
  }
  _inn.push_str(path);
}

auto Path::pop() -> bool {
  auto cmpts = Components{_inn};

  const auto tail = cmpts.next_back();
  if (tail.is_empty()) {
    return false;
  }

  _inn.truncate(cmpts.as_str().len());
  return true;
}

void Path::set_file_name(Str file_name) {
  const auto old_name = this->file_name();
  if (old_name.is_empty()) {
    return;
  }
  this->pop();
  this->push(file_name);
}


auto Path::exists() const -> bool {
  return fs::meta(*this).is_ok();
}

auto Path::is_file() const -> bool {
  const auto t = fs::meta(*this).ok();
  return t && (*t).is_file();
}

auto Path::is_dir() const -> bool {
  const auto t = fs::meta(*this).ok();
  return t && (*t).is_dir();
}

}  // namespace sfc::fs
