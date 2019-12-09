#include "rc.inl"

#include "rc/fs/path.h"
#include "rc/sys.h"

namespace rc::fs {

#pragma region Path
pub auto Path::parent() const -> Path {
  if (_inner.is_empty()) return Path{u8""};
  const auto xpos = _inner.find(u8'/').unwrap_or(0);
  return _inner.slice_unchecked(0, xpos);
}

pub auto Path::file_name() const -> Str {
  if (_inner.is_empty()) return u8"";
  const auto pos = (_inner.rfind(u8'/') + 1).unwrap_or(0);
  return _inner.slice_from(pos);
}

pub auto Path::file_stem() const -> Str {
  const auto name = this->file_name();
  const auto xpos = name.rfind(u8'.').unwrap_or(name.len());
  return name.slice_unchecked(0, xpos);
}

pub auto Path::extension() const -> Str {
  const auto name = this->file_name();
  const auto xpos = name.rfind(u8'.').unwrap_or(name.len());
  return name.slice_unchecked(xpos, name.len());
}

pub auto Path::exists() const -> bool {
  const auto ret = sys::fs::exists(_inner);
  return ret;
}

pub auto Path::is_file() const -> bool {
  const auto ret = sys::fs::is_file(_inner);
  return ret;
}

pub auto Path::is_dir() const -> bool {
  const auto ret = sys::fs::is_dir(_inner);
  return ret;
}
#pragma endregion

#pragma region PathBuf

pub PathBuf::PathBuf(Str s) : _inner{string::String::from(s)} {}

pub auto PathBuf::as_str() const noexcept -> Str { return _inner.as_str(); }

pub auto PathBuf::as_path() const noexcept -> Path {
  return Path{_inner.as_str()};
}

pub auto PathBuf::clone() const -> PathBuf { return PathBuf{_inner.as_str()}; }

pub auto PathBuf::exists() const -> bool {
  return sys::fs::exists(_inner.as_str());
}

pub auto PathBuf::is_file() const -> bool {
  return sys::fs::exists(_inner.as_str());
}

pub auto PathBuf::is_dir() const -> bool {
  return sys::fs::is_dir(_inner.as_str());
}

pub auto PathBuf::parent() const -> Path { return this->as_path().parent(); }

pub auto PathBuf::file_name() const -> Str {
  return this->as_path().file_name();
}

pub auto PathBuf::file_stem() const -> Str {
  return this->as_path().file_stem();
}

pub auto PathBuf::extension() const -> Str {
  return this->as_path().extension();
}

pub auto PathBuf::push(Str s) -> void {
  if (!_inner.is_empty() && !_inner.ends_with(u8'/')) {
    _inner.push(u8'/');
  }
  if (s.starts_with(u8'/')) {
    s = s.slice_from(1);
  }
  _inner.push_str(s);
}

pub auto PathBuf::pop() -> void {
  const auto idx = _inner.rfind(u8'/').unwrap_or(0);
  _inner.truncate(idx);
}

pub auto PathBuf::set_file_name(Str new_name) -> void {
  const auto old_name = this->file_name();
  _inner.truncate(_inner.len() - old_name.len());
  _inner.push_str(new_name);
}

pub auto PathBuf::set_extension(Str new_ext) -> void {
  const auto old_ext = this->extension();
  _inner.truncate(_inner.len() - old_ext.len());
  _inner.push_str(new_ext);
}
#pragma endregion

#pragma region funcs
pub auto exists(Path p) -> bool {
  // redirect to sys::fs
  return sys::fs::exists(p._inner);
}

pub auto is_dir(Path p) -> bool {
  // redirect to sys::fs
  return sys::fs::is_dir(p._inner);
}

pub auto is_file(Path p) -> bool {
  // redirect to sys::fs
  return sys::fs::is_file(p._inner);
}

pub auto copy(Path from, Path to) -> void {
  // redirect to sys::fs
  return sys::fs::copy(from._inner, to._inner);
}

pub auto rename(Path from, Path to) -> void {
  // redirect to sys::fs
  return sys::fs::rename(from._inner, to._inner);
}

pub auto create_dir(Path p) -> void {
  // redirect to sys::fs
  return sys::fs::create_dir(p._inner);
}

pub auto remove_file(Path p) -> void {
  // redirect to sys::fs
  return sys::fs::remove_file(p._inner);
}

pub auto remove_dir(Path p) -> void {
  // redirect to sys::fs
  return sys::fs::remove_dir(p._inner);
}
#pragma endregion

}  // namespace rc::fs
