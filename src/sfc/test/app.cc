#include "sfc/test/app.h"

#include "sfc/app/opts.h"
#include "sfc/fs.h"
#include "sfc/io.h"
#include "sfc/time.h"

namespace sfc::test {

using collections::VecMap;

static auto parse_color(Str color_str) -> bool {
  if (color_str == "auto") {
    return io::Stdout::is_tty();
  } else if (color_str == "on") {
    return true;
  } else if (color_str == "off") {
    return false;
  }
  return false;
}

static auto parse_gtest_filter(Str pats) -> Vec<Str> {
  auto v = Vec<Str>{};
  for (auto s = pats; s;) {
    auto p = 0U;
    for (; p < s._len; p++) {
      if (s[p] != ':') {
        continue;
      }
      if (s[p + 1] == ':') {
        p += 1;
      }
    }
    v.push(s);
    s = s[{p + 1, _}];
  }
  return v;
}

auto App::run(Slice<const Str> args) -> int {
  auto opts = app::Opts{};
  opts.add_opt("help", "Show this help message", 'h');
  opts.add_opt("list", "List all tests", 'l');
  opts.add_opt("gtest_list_tests", "List all tests instead of running them");
  opts.add_opt("gtest_filter", "Run only tests matching the given pattern", 'f');
  opts.add_opt("gtest_color", "Enable or disable colored output", 'c', "auto");
  opts.add_opt("gtest_output", "Output file for test results (default: stdout)", 'o', "stdout");

  opts.parse(args);

  if (opts.has_flag("help")) {
    this->help();
    return 0;
  }

  if (opts.has_flag("list")) {
    io::println(this->list_tests());
    return 0;
  }

  if (opts.has_flag("gtest_list_tests")) {
    const auto path = opts.get("gtest_output");
    this->list_to_file(path);
    return 0;
  }

  const auto gtest_filter = opts.get("gtest_filter");
  const auto gtest_color = opts.get("gtest_color");

  const auto filters = parse_gtest_filter(gtest_filter);
  const auto color_val = parse_color(gtest_color);
  this->run_tests(filters.as_slice(), color_val);

  return 0;
}

void App::help() {
  static const char msg[] =
      "This program contains tests written using SFC Test. You can use the\n"
      "following command line flags to control its behavior: \n"
      "\n"
      "Test Selection:\n"
      "  --gtest_list_tests\n"
      "      List the names of all tests instead of running them.\n"
      "  --gtest_filter=POSITIVE_PATTERNS\n"
      "      Run only the tests whose name starts with the pattern.";
  io::println(msg);
}

void App::run_tests(Slice<const Str> pats, bool color) {
  auto& tester = Tester::instance();
  tester.invoke(pats, color);
}

void App::list_to_file(Str output) const {
  const auto file_indx = output.find(':');
  const auto file_path = file_indx ? output[{*file_indx + 1, _}] : output;

  const auto file_text = this->list_tests_xml();
  if (file_path) {
    auto file = fs::File::create(fs::Path::from(file_path)).unwrap();
    file.write_str(file_text);
  } else {
    io::print(file_text);
  }
}

auto App::list_tests() const -> String {
  auto suites = Tester::instance().suites();

  auto sbuf = String{};
  auto fmts = fmt::Fmter<String>{sbuf};
  for (const auto& suite : suites) {
    fmts.write_fmt("{}\n", suite.name());
    for (const auto& test : suite.tests()) {
      fmts.write_fmt("  {}\n", test.name());
    }
  }
  return sbuf;
}

auto App::list_tests_xml() const -> String {
  auto suites = Tester::instance().suites();

  auto test_cnt = 0U;
  for (const auto& suite : suites) {
    test_cnt += suite.tests().len();
  }

  auto sbuf = String{};
  auto fmts = fmt::Fmter<String>{sbuf};
  fmts.write_str("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  fmts.write_fmt("<testsuites tests=\"{}\" name=\"AllTests\">\n", test_cnt);
  for (auto& suite : suites) {
    auto tests = suite.tests();
    fmts.write_fmt("  <testsuite name=\"{}\" tests=\"{}\">\n", suite.name(), tests.len());
    for (auto& test : tests) {
      fmts.write_fmt("    <testcase name=\"{}\" file=\"{}\" line=\"{}\" />\n",
                     test.name(),
                     Str::from(test._loc.file),
                     test._loc.line);
    }
    fmts.write_str("  </testsuite>\n");
  }
  fmts.write_str("</testsuites>\n");
  return sbuf;
}

}  // namespace sfc::test
