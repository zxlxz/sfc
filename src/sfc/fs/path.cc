#include "sfc/fs/path.h"

#include "sfc/sys/fs.h"

namespace sfc::fs {

namespace sys_imp = sys::fs;

struct Components {
  Str _str;

 public:
  auto as_str() const noexcept -> Str {
    return _str;
  }

  auto next_back() noexcept -> Str {
    while (!_str.is_empty()) {
      _str = _str.trim_end_matches('/');

      if (const auto p = _str.rfind('/')) {
        const auto b = _str[{*p + 1, $}];
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

auto Path::as_str() const noexcept -> Str {
  return _inn;
}

auto Path::file_name() const noexcept -> Str {
  const auto s = Components{_inn.as_str()}.next_back();
  if (s == "." || s == ".." || s == "/") {
    return {};
  }
  return s;
}

auto Path::extension() const noexcept -> Str {
  const auto file_name = this->file_name();
  const auto dot_pos = file_name.rfind('.').unwrap_or(file_name.len());
  return file_name[{dot_pos + 1, $}];
}

auto Path::file_stem() const noexcept -> Str {
  const auto file_name = this->file_name();
  const auto dot_pos = file_name.rfind('.').unwrap_or(file_name.len());
  return file_name[{0, dot_pos}];
}

auto Path::parent() const noexcept -> Path {
  auto cmpts = Components{_inn.as_str()};
  cmpts.next_back();

  return Path{cmpts.as_str()};
}

auto Path::join(Str path) const noexcept -> PathBuf {
  auto res = PathBuf::from(_inn);
  res.push(path);
  return res;
}

auto Path::is_absolute() const noexcept -> bool {
  return _inn.starts_with('/');
}

auto Path::is_relative() const noexcept -> bool {
  return !_inn.starts_with('/');
}

auto Path::exists() const noexcept -> bool {
  const auto m = fs::meta(*this);
  return m.is_ok();
}

auto Path::is_file() const noexcept -> bool {
  const auto t = fs::meta(*this).ok();
  return t && (*t).is_file();
}

auto Path::is_dir() const noexcept -> bool {
  const auto t = fs::meta(*this).ok();
  return t && (*t).is_dir();
}

auto PathBuf::from(Str s) noexcept -> PathBuf {
  auto res = PathBuf{};
  res._inn.push_str(s);
  return res;
}

auto PathBuf::as_path() const noexcept -> Path {
  return Path{_inn.as_str()};
}

auto PathBuf::as_str() const noexcept -> Str {
  return _inn.as_str();
}

void PathBuf::clear() noexcept {
  _inn.clear();
}

void PathBuf::reserve(usize additional) noexcept {
  _inn.reserve(additional);
}

void PathBuf::push(Str path) noexcept {
  if (!_inn.ends_with('/')) {
    _inn.push('/');
  }
  if (path.starts_with('/')) {
    _inn.clear();
  }
  _inn.push_str(path);
}

auto PathBuf::pop() noexcept -> bool {
  auto cmpts = Components{_inn.as_str()};

  const auto tail = cmpts.next_back();
  if (tail.is_empty()) {
    return false;
  }

  _inn.truncate(cmpts.as_str().len());
  return true;
}

void PathBuf::set_file_name(Str new_file_name) noexcept {
  const auto old_name = this->as_path().file_name();
  if (!old_name.is_empty()) {
    _inn.truncate(_inn.len() - old_name.len());
  }
  this->push(new_file_name);
}

void PathBuf::set_extension(Str new_ext) noexcept {
  const auto old_path = this->as_path();
  const auto file_stem = old_path.file_stem();
  if (file_stem.is_empty()) {
    return;
  }

  const auto end_stem = file_stem.as_ptr() + file_stem.len();
  _inn.truncate(static_cast<usize>(end_stem - _inn.as_ptr()));

  if (!new_ext.is_empty()) {
    _inn.reserve(new_ext.len() + 1);
    _inn.push('.');
    _inn.push_str(new_ext);
  }
}

auto Meta::exists() const noexcept -> bool {
  return _attr != 0;
}

auto Meta::file_len() const noexcept -> u64 {
  return _size;
}

auto Meta::is_dir() const noexcept -> bool {
  return sys_imp::is_dir(_attr);
}

auto Meta::is_file() const noexcept -> bool {
  return sys_imp::is_file(_attr);
}

auto meta(Path path) -> io::Result<Meta> {
  const auto c_path = CString::from(path.as_str());

  auto res = Meta{};
  if (!sys_imp::lstat(c_path, res)) {
    return io::last_os_error();
  }
  return res;
}

auto create_dir(Path path) -> io::Result<> {
  const auto c_path = CString::from(path.as_str());

  if (sys_imp::mkdir(c_path)) {
    return {};
  }
  const auto err = io::last_os_error();
  if (err != io::Error::AlreadyExists) {
    return err;
  }
  return {};
}

auto remove_dir(Path path) -> io::Result<> {
  const auto c_path = CString::from(path.as_str());

  const auto ret = sys_imp::rmdir(c_path);
  if (!ret) {
    return io::last_os_error();
  }

  return {};
}

auto remove_file(Path path) -> io::Result<> {
  const auto c_path = CString::from(path.as_str());

  const auto ret = sys_imp::unlink(c_path);
  if (!ret) {
    return io::last_os_error();
  }

  return {};
}

auto rename(Path old_path, Path new_path) -> io::Result<> {
  const auto c_old = CString::from(old_path.as_str());
  const auto c_new = CString::from(new_path.as_str());

  const auto ret = sys_imp::rename(c_old, c_new);
  if (!ret) {
    return io::last_os_error();
  }

  return {};
}

}  // namespace sfc::fs
