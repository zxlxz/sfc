#include "rc.inl"

#include "rc/fs/file.h"

namespace rc::fs {

#pragma region OpenOptions
pub auto OpenOptions::read(bool value) -> OpenOptions& {
  _inner._read = value;
  return *this;
}

pub auto OpenOptions::write(bool value) -> OpenOptions& {
  _inner._write = value;
  return *this;
}

pub auto OpenOptions::truncate(bool value) -> OpenOptions& {
  _inner._truncate = value;
  return *this;
}

pub auto OpenOptions::create(bool value) -> OpenOptions& {
  _inner._create = value;
  return *this;
}

pub auto OpenOptions::create_new(bool value) -> OpenOptions& {
  _inner._create_new = value;
  return *this;
}

pub auto OpenOptions::open(Path path) const -> sys::fs::File {
  return _inner.open(path._inner);
}
#pragma endregion

#pragma region File
pub auto File::create(Path p) -> File {
  const auto opts = OpenOptions{}.read(true);
  return File{opts.open(p)};
}

pub auto File::open(Path p) -> File {
  const auto opts = OpenOptions{}.write(true).create(true).truncate(true);
  return File{opts.open(p)};
}

pub auto File::size() const -> usize {
  const auto ret = _inner.size();
  return ret;
}

pub auto File::seek(SeekFrom pos) -> usize {
  const auto ret = _inner.seek(pos);
  return ret;
}

pub auto File::read(Slice<u8> buf) -> usize {
  const auto ret = _inner.read(buf);
  return ret;
}

pub auto File::write(Slice<const u8> buf) -> usize {
  const auto ret = _inner.write(buf);
  return ret;
}
#pragma endregion

}  // namespace rc::fs
