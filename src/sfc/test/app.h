#pragma once

#include "unit.h"

namespace sfc::test {

class App {
 public:
  App();
  ~App();
  App(App&&) noexcept;

  auto run(Slice<const Str> args) -> int;
  void help();

 private:
  void run_tests(Str filter, bool color);
  void list_to_file(Str path);

  auto list_tests(Str fmt) -> String;
  auto list_tests_xml() -> String;
};

}  // namespace sfc::test
