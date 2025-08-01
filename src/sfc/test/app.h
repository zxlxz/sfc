#pragma once

#include "sfc/test/unit.h"

namespace sfc::test {

class App {
 public:
  auto run(Slice<const Str> args) -> int;
  void help();

 private:
  void run_tests(Slice<const Str> pats, bool color);
  void list_to_file(Str output) const;

  auto list_tests() const -> String;
  auto list_tests_xml() const -> String;
};

}  // namespace sfc::test
