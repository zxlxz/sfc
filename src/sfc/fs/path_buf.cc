#include "path_buf.h"

namespace sfc::fs {

PathBuf::PathBuf(String inn) : _inn{mem::move(inn)} {}

PathBuf::PathBuf() = default;

PathBuf::~PathBuf() = default;

PathBuf::PathBuf(PathBuf&&) noexcept = default;

PathBuf& PathBuf::operator=(PathBuf&&) noexcept = default;

auto PathBuf::from(Path path) -> PathBuf {
  return PathBuf{String::from(path.as_str())};
}

auto PathBuf::clone() const -> PathBuf {
  return PathBuf{_inn.clone()};
}

void PathBuf::push(Path path) {
  const auto p = path.as_str();

  if (!_inn.ends_with('/')) {
    _inn.push('/');
  }
  if (p.starts_with('/')) {
    _inn.clear();
  }
  _inn.push_str(path.as_str());
}

auto PathBuf::pop() -> bool {
  const auto p = this->as_path().parent().as_str();
  if (!p) {
    return false;
  }

  _inn.truncate(p.len());
  return true;
}

void PathBuf::set_file_name(Str file_name) {
  const auto old_name = this->as_path().file_name();
  if (old_name) {
    this->pop();
  }
  this->push(file_name);
}

}  // namespace sfc::fs
