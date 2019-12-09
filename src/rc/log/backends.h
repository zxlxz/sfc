#pragma once

#include "rc/fs.h"
#include "rc/io.h"
#include "rc/log/mod.h"

namespace rc::log {

struct Stdout {
  pub auto push(const Entry& entry) -> void;
};

struct File {
  using Inner = io::BufWriter<fs::File>;
  Inner _inner;

  pub static auto create(fs::Path p) -> File;
  pub auto push(const Entry& entry) -> void;
};

}  // namespace rc::log
