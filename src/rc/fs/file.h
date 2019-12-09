#pragma once

#include "rc/fs/mod.h"
#include "rc/fs/path.h"
#include "rc/io.h"

#include "rc/sys.h"

namespace rc::fs {

struct OpenOptions {
  sys::fs::OpenOptions _inner;

  pub auto read(bool value) -> OpenOptions& ;
  pub auto write(bool value) -> OpenOptions&;
  pub auto truncate(bool value) -> OpenOptions& ;
  pub auto create(bool value) -> OpenOptions&;
  pub auto create_new(bool value) -> OpenOptions&;
  pub auto open(Path path) const -> sys::fs::File;
};

struct File  {
  sys::fs::File _inner;

  pub static auto create(Path p) -> File;
  pub static auto open(Path p) -> File;

  pub auto size() const -> usize;
  pub auto seek(SeekFrom pos) -> usize;
  pub auto read(Slice<u8> buf) -> usize;
  pub auto write(Slice<const u8> buf) -> usize;
};

}  // namespace rc::fs
