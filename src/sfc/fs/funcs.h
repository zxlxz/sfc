
#pragma once

#include "path.h"

namespace sfc::fs {

auto create_dir(Path path) -> io::Result<>;

auto remove_dir(Path path) -> io::Result<>;

auto remove_file(Path path) -> io::Result<>;

auto rename(Path from, Path to) -> io::Result<>;

auto symlink(Path original, Path link) -> io::Result<>;

}  // namespace sfc::fs
