#pragma once

#include "path.h"

namespace sfc::fs {

class PathBuf {
  String _inn;

 public:
  PathBuf();
  ~PathBuf();

  explicit PathBuf(String inn);
  PathBuf(PathBuf&&) noexcept;

  PathBuf& operator=(PathBuf&&) noexcept;

  static auto from(Path path) -> PathBuf;

  auto clone() const -> PathBuf;

  auto as_path() const -> Path {
    return Path{_inn.as_str()};
  }

 public:
  // 'a'.push('b') -> 'a/b'
  void push(Path path);

  // 'a/b'.pop() -> 'a'
  auto pop() -> bool;

  void set_file_name(Str s);
};

inline auto Path::join(const auto& p) const -> PathBuf {
  auto res = PathBuf::from(*this);
  res.push(p);
  return res;
}

inline auto Path::operator/(const auto& p) const -> PathBuf {
  auto res = PathBuf::from(*this);
  res.push(p);
  return res;
}

}  // namespace sfc::fs
