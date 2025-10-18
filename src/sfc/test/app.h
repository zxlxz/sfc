#pragma once

#include "sfc/test/case.h"

namespace sfc::test {

class App {
 public:
  void help();
  void list() const;
  void list_xml(Str path) const;
  void exec(Str filters, Option<bool> color = {});
};

void main(int argc, const char* argv[]);

}  // namespace sfc::test
