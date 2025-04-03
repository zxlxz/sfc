
#pragma once

#include "path.h"

namespace sfc::fs {

auto create_dir(const Path& path) -> io::Result<>;

auto remove_dir(const Path& path) -> io::Result<>;

auto remove_file(const Path& path) -> io::Result<>;

auto rename(const Path& from, const Path& to) -> io::Result<>;

}  // namespace sfc::fs
