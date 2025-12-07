#pragma once

#include "sfc/core.h"

namespace sfc::test {

class App {
 public:
  void help();
  void list() const;
  void list_xml(Str path) const;
  void exec(Str filters, Option<bool> color = {});
};

auto main(int argc, const char* argv[]) -> int;

}  // namespace sfc::test
