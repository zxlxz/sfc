#if defined(__unix__) || defined(__APPLE__)
#include "sfc/sys/unix/fs.inl"
#elif defined(_WIN32)
#include "sfc/sys/windows/fs.inl"
#endif

#include "sfc/fs/path.h"
#include "sfc/ffi/os_str.h"

namespace sfc::fs {

struct Components {
  Str _str;

 public:
  auto len() const -> usize {
    return _str.len();
  }

  static constexpr auto is_delim(char c) -> bool {
#ifdef _WIN32
    return c == '/' || c == '\\';
#else
    return c == '/';
#endif
  };

  auto next() noexcept -> Option<Str> {
    if (_str.is_empty()) {
      return {};
    }

    if (_str.len() == 1) {
      return mem::take(_str);
    }

    const auto p = _str.find(is_delim);
    if (!p) {
      return mem::take(_str);
    }

    // root, use '/' for all platforms, don't use ugly '\'
    if (*p == 0) {
      _str = _str[{1, $}];
      return Str{"/"};
    }

    const auto s = _str[{0, *p}];
    _str = _str[{*p + 1, $}];
    return s;
  }

  auto next_back() noexcept -> Option<Str> {
    if (_str.is_empty()) {
      return {};
    }

    // trim delimiters
    if (_str.len() > 1 && _str.ends_with(is_delim)) {
      _str._len -= 1;
    }

    if (_str.len() == 1) {
      return mem::take(_str);
    }

    const auto p = _str.rfind(is_delim);
    if (!p) {
      return mem::take(_str);
    }

    const auto i = *p;
    const auto s = _str[{i + 1, $}];
    _str = _str[{0, i ? i : 1}];
    return s;
  }
};

auto Path::as_str() const noexcept -> Str {
  return _inn;
}

auto Path::file_name() const noexcept -> Str {
  if (_inn.is_empty()) {
    return {};
  }

  auto cmpts = Components{_inn};
  const auto s = cmpts.next_back().unwrap_or("");
  if (s == "" || s == "." || s == ".." || s == "/") {
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
  auto cmpts = fs::Components{_inn.as_str()};
  cmpts.next_back();

  return Path{cmpts._str};
}

auto Path::join(Str path) const noexcept -> PathBuf {
  auto res = PathBuf::from(_inn);
  res.push(Path{path});
  return res;
}

auto Path::is_root() const noexcept -> bool {
  if (_inn.is_empty()) {
    return false;
  }
  if (_inn == "/") {
    return true;
  }
#ifdef _WIN32
  if (_inn.len() == 2 && _inn.ends_with(':')) {
    return true;
  }
#endif
  return false;
}

auto Path::is_absolute() const noexcept -> bool {
  if (_inn.is_empty()) {
    return false;
  }

  if (_inn.starts_with('/')) {
    return true;
  }

#ifdef _WIN32
  const auto first_cmpt = Path{Components{_inn}.next().unwrap_or("")};
  if (first_cmpt.is_root()) {
    return true;
  }
#endif
  return false;
}

auto Path::is_relative() const noexcept -> bool {
  if (_inn.is_empty()) {
    return false;
  }
  return !this->is_absolute();
}

auto Path::exists() const noexcept -> bool {
  const auto m = fs::metadata(*this);
  return m.is_ok();
}

auto Path::is_file() const noexcept -> bool {
  const auto t = fs::metadata(*this).ok();
  return t && (*t).is_file();
}

auto Path::is_dir() const noexcept -> bool {
  const auto t = fs::metadata(*this).ok();
  return t && (*t).is_dir();
}

auto PathBuf::from(Str s) noexcept -> PathBuf {
  auto res = PathBuf{};
  res.push(Path{s});
  return res;
}

auto PathBuf::as_path() const noexcept -> Path {
  return Path{_inn.as_str()};
}

auto PathBuf::as_str() const noexcept -> Str {
  return _inn.as_str();
}

auto PathBuf::clone() const -> PathBuf {
  auto res = PathBuf{};
  res._inn = _inn.clone();
  return res;
}

void PathBuf::clear() noexcept {
  _inn.clear();
}

void PathBuf::reserve(usize additional) noexcept {
  _inn.reserve(additional);
}

void PathBuf::push(Path path) noexcept {
  if (path.is_absolute()) {
    _inn.clear();
  }

  auto cmpts = Components{path._inn};
  while (auto x = cmpts.next()) {
    const auto s = *x;
    // skip empty or '.', but don't normalize '..'
    if (s == "" || s == ".") {
      continue;
    }
    // use '/' for all platforms!, don't use ugly '\'
    if (!_inn.is_empty() && !_inn.ends_with('/')) {
      _inn.push('/');
    }
    _inn.push_str(s);
  }
}

auto PathBuf::pop() noexcept -> bool {
  if (_inn.is_empty()) {
    return false;
  }

  auto cmpts = Components{_inn.as_str()};
  while (auto s = cmpts.next_back()) {
    if (!s->is_empty()) {
      break;
    }
  }

  _inn.truncate(cmpts.len());
  return true;
}

void PathBuf::set_file_name(Str new_file_name) noexcept {
  const auto old_name = this->as_path().file_name();
  if (!old_name.is_empty()) {
    _inn.truncate(_inn.len() - old_name.len());
  }
  this->push(Path{new_file_name});
}

void PathBuf::set_extension(Str new_ext) noexcept {
  const auto old_path = this->as_path();
  const auto file_stem = old_path.file_stem();
  if (file_stem.is_empty()) {
    return;
  }

  // file_stem and old_path, based on same str, so we can caculate ptrdiff
  const auto stem_pos = static_cast<usize>(file_stem.as_ptr() - old_path.as_str().as_ptr());
  _inn.truncate(stem_pos + file_stem.len());

  if (!new_ext.is_empty()) {
    _inn.reserve(new_ext.len() + 1);
    _inn.push('.');
    _inn.push_str(new_ext);
  }
}

auto PathBuf::join(Path path) const -> PathBuf {
  auto res = this->clone();
  res.push(path);
  return res;
}

auto Metadata::exists() const noexcept -> bool {
  return _attr != 0;
}

auto Metadata::file_len() const noexcept -> u64 {
  return _size;
}

auto Metadata::is_dir() const noexcept -> bool {
  return sys::Metadata{_attr, _size}.is_dir();
}

auto Metadata::is_file() const noexcept -> bool {
  return sys::Metadata{_attr, _size}.is_file();
}

auto metadata(Path path) -> io::Result<Metadata> {
  const auto os_path = ffi::OsString::from(path.as_str());
  const auto sys_meta = _TRY(sys::lstat(os_path.ptr()));
  return Metadata{sys_meta._attr, sys_meta._size};
}

auto create_dir(Path path) -> io::Result<> {
  if (path._inn.is_empty() || path.is_root()) {
    return io::Error::InvalidInput;
  }

  const auto os_path = ffi::OsString::from(path.as_str());
  return sys::mkdir(os_path.ptr());
}

auto create_dir_all(Path path) -> io::Result<> {
  const auto err = fs::create_dir(path).err();
  if (!err || *err == io::Error::AlreadyExists) {
    return {};
  }

  const auto parent = path.parent();
  _TRY(fs::create_dir_all(parent));

  return fs::create_dir(path);
}

auto remove_dir(Path path) -> io::Result<> {
  const auto os_path = ffi::OsString::from(path.as_str());
  return sys::rmdir(os_path.ptr());
}

auto remove_file(Path path) -> io::Result<> {
  const auto os_path = ffi::OsString::from(path.as_str());
  return sys::unlink(os_path.ptr());
}

auto rename(Path old_path, Path new_path) -> io::Result<> {
  const auto os_old = ffi::OsString::from(old_path.as_str());
  const auto os_new = ffi::OsString::from(new_path.as_str());
  return sys::rename(os_old.ptr(), os_new.ptr());
}

}  // namespace sfc::fs
