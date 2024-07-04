#include "sfc/fs/path.h"

#include "sfc/sys/fs.h"

namespace sfc::fs {

struct Components {
  Str _str;

 public:
  auto as_str() const -> Str {
    return _str;
  }

  auto next_back() -> Str {
    while (!_str.is_empty()) {
      _str = _str.trim_end_matches('/');

      if (const auto p = _str.rfind('/')) {
        const auto b = _str[{*p + 1, _}];
        _str = _str[{0, *p}];
        if (b != ".") {
          return b;
        }
      } else {
        return mem::take(_str);
      }
    }

    return {};
  }
};

namespace sys_imp = sys::fs;

Path::Path() = default;

Path::~Path() = default;

Path::Path(Path&&) noexcept = default;

auto Path::operator=(Path&&) noexcept -> Path& = default;

auto Path::from(Str s) -> Path {
  auto res = Path{};
  res._inn.push_str(s);
  return res;
}

auto Path::clone() const -> Path {
  return Path::from(_inn.as_str());
}

auto Path::as_str() const -> Str {
  return _inn.as_str();
}

auto Path::file_name() const -> Str {
  const auto s = Components{_inn.as_str()}.next_back();
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
  auto cmpts = Components{_inn.as_str()};
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
  auto cmpts = Components{_inn.as_str()};

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
  const auto m = fs::meta(*this);
  return m.is_ok();
}

auto Path::is_file() const -> bool {
  const auto t = fs::meta(*this).ok();
  return t && (*t).is_file();
}

auto Path::is_dir() const -> bool {
  const auto t = fs::meta(*this).ok();
  return t && (*t).is_dir();
}

auto Meta::exists() const -> bool {
  return _attr != 0;
}

auto Meta::file_len() const -> u64 {
  return _size;
}

auto Meta::is_dir() const -> bool {
  return sys_imp::is_dir(_attr);
}

auto Meta::is_file() const -> bool {
  return sys_imp::is_file(_attr);
}

auto meta(const Path& path) -> io::Result<Meta> {
  const auto c_path = CString::from(path.as_str());

  auto res = Meta{};
  if (!sys_imp::lstat(c_path, res)) {
    return io::Error::last_os_error();
  }
  return res;
}

auto create_dir(const Path& path) -> io::Result<> {
  const auto c_path = CString::from(path.as_str());
  const auto ret = sys_imp::mkdir(c_path);

  if (!ret) {
    return io::Error::last_os_error();
  }

  return _;
}

auto remove_dir(const Path& path) -> io::Result<> {
  const auto c_path = CString::from(path.as_str());

  const auto ret = sys_imp::rmdir(c_path);
  if (!ret) {
    return io::Error::last_os_error();
  }

  return _;
}

auto remove_file(const Path& path) -> io::Result<> {
  const auto c_path = CString::from(path.as_str());

  const auto ret = sys_imp::unlink(c_path);
  if (!ret) {
    return io::Error::last_os_error();
  }

  return _;
}

auto rename(const Path& old_path, const Path& new_path) -> io::Result<> {
  const auto c_old = CString::from(old_path.as_str());
  const auto c_new = CString::from(new_path.as_str());

  const auto ret = sys_imp::rename(c_old, c_new);
  if (!ret) {
    return io::Error::last_os_error();
  }

  return _;
}

}  // namespace sfc::fs
