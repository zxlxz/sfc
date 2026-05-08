#pragma once

#include "sfc/alloc.h"

namespace sfc::test {

void gtest_help();
void gtest_list_tests(Str path);
void gtest_run(Str filter, bool color);

}  // namespace sfc::test
