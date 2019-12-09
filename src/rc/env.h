#pragma once

#include "rc/fs/path.h"

namespace rc::env {

pub auto var(Str key) -> String;
pub auto home_dir() -> fs::PathBuf;
pub auto current_dir() -> fs::PathBuf;
pub auto current_exe() -> fs::PathBuf;
pub auto set_current_dir(fs::Path p) -> void;

struct Args {

};

}  // namespace rc::env
