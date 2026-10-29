#pragma once

#include "sfc/alloc.h"

namespace sfc::test {

struct GTestSuite;

class GTestApp {
  List<GTestSuite> _suites{};
  usize _case_cnt{};
  usize _run_cnt{};
  usize _passed_cnt{};
  usize _failed_cnt{};
  usize _duration_ms{};

 public:
  GTestApp();
  ~GTestApp();

 public:
  static void show_help();
  static void list(Str uri);

  void filt(Str filter);
  auto run(bool color) -> bool;

 private:
  static auto split_pats(Str filter) -> List<Str>;
  static auto list_xml() -> String;

  void on_run_start(bool use_color) const;
  void on_run_end(bool use_color) const;
  void show_summary(bool use_color) const;
};

}  // namespace sfc::test
