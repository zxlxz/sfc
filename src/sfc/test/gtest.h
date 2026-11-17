#pragma once

#include "sfc/alloc.h"

namespace sfc::test {

struct GTestSuite;

class GTestApp {
  List<GTestSuite> _suites;
  usize _case_cnt{0};
  usize _run_cnt{0};
  usize _passed_cnt{0};
  usize _failed_cnt{0};
  usize _duration_ms{0};

 public:
  GTestApp();
  ~GTestApp();

  GTestApp(const GTestApp&) = delete;
  GTestApp& operator=(const GTestApp&) = delete;

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
