#pragma once

#include "sfc/alloc.h"

namespace sfc::test {

void gtest_help();
void gtest_list(Str uri);
auto gtest_run(Str pattern, bool use_color) -> int;

}  // namespace sfc::test
