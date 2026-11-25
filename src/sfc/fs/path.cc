#include "sfc/fs/path.h"
#include "sfc/fs/file.h"

namespace sfc::fs {

namespace {

constexpr auto is_path_delim(char c) -> bool {
#ifdef _WIN32
  return c == '/' || c == '\\';
#else
  return c == '/';
#endif
};

struct Components {
  Str _str;

 public:
  auto next_back() noexcept -> Option<Str> {
    _str = _str.trim_end_matches(is_path_delim);
    if (_str.is_empty()) {
      return {};
    }

    const auto pos = _str.rfind(is_path_delim);
    if (!pos) {
      const auto ret = _str;
      _str = {};
      return ret;
    }

    const auto ret = _str[{*pos + 1, $}];
    const auto head = _str[{0, *pos + 1}].trim_end_matches(is_path_delim);
    _str = head.is_empty() ? Str{"/"} : head;
    return ret;
  }
};

struct FileName {
  Str _str;

 public:
  auto is_empty() const noexcept -> bool {
    return _str.is_empty();
  }

  auto dot_pos() const noexcept -> usize {
    return _str.rfind('.').unwrap_or(_str.len());
  }

  auto file_stem() const noexcept -> Str {
    const auto p = this->dot_pos();
    return _str[{0, p}];
  }

  auto extension() const noexcept -> Str {
    const auto p = this->dot_pos();
    return _str[{p + 1, $}];
  }
};

}  // namespace

auto Path::as_str() const noexcept -> Str {
  return _inn;
}

auto Path::file_name() const noexcept -> Str {
  if (_inn.is_empty()) {
    return {};
  }

  auto cmpts = Components{_inn.as_str()};
  const auto last = cmpts.next_back().unwrap_or("");
  if (last == "" || last == "." || last == ".." || last == "/") {
    return {};
  }
  return last;
}

auto Path::extension() const noexcept -> Str {
  const auto file_name = FileName{this->file_name()};
  return file_name.extension();
}

auto Path::file_stem() const noexcept -> Str {
  const auto file_name = FileName{this->file_name()};
  return file_name.file_stem();
}

auto Path::parent() const noexcept -> Path {
  auto cmpts = fs::Components{_inn};
  cmpts.next_back();

  return Path{cmpts._str};
}

auto Path::is_root() const noexcept -> bool {
  if (_inn.is_empty()) {
    return false;
  }

  if (_inn == "/") {
    return true;
  }

  return false;
}

auto Path::is_absolute() const noexcept -> bool {
  if (_inn.is_empty()) {
    return false;
  }

  if (is_path_delim(_inn[0])) {
    return true;
  }

#ifdef _WIN32
  // match [a-z]:
  if (_inn[1] == ':' && chr::is_alpha(_inn[0])) {
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
  const auto meta = fs::metadata(_inn).ok();
  return bool(meta);
}

auto Path::is_file() const noexcept -> bool {
  const auto meta = fs::metadata(_inn).ok();
  return meta && meta->is_file();
}

auto Path::is_dir() const noexcept -> bool {
  const auto meta = fs::metadata(_inn).ok();
  return meta && meta->is_dir();
}

auto Path::operator==(const Path& p) const noexcept -> bool {
  return _inn == p._inn;
}

void Path::fmt(fmt::Formatter& f) const {
  _inn.fmt(f);
}

}  // namespace sfc::fs
