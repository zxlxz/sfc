#include "sfc/fs.h"
#include "sfc/io.h"
#include "sfc/app/clap.h"
#include "sfc/test/app.h"
#include "sfc/test/ut.h"

namespace sfc::test {

static auto parse_filter(Str filter) -> Vec<Str> {
  auto find_pos = [](Str s) -> usize {
    for (auto p = 0U; p < s._len; p++) {
      switch (s[p]) {
        case ';': return p;
        case ':':
          if (s[p + 1] != ':') {
            return p;
          }
          p++;
      }
    }
    return s._len;
  };

  auto v = Vec<Str>{};
  while (!filter.is_empty()) {
    const auto pos = find_pos(filter);
    v.push(filter[{0, pos}]);
    filter = filter[{pos + 1, $}];
  }
  return v;
}

static auto format_xml(Slice<const Suite> suites) -> String {
  auto all_cnt = usize{0};
  for (const auto& suite : suites) {
    all_cnt += suite.tests().len();
  }

  auto buf = String{};
  auto out = fmt::Fmter{buf};
  out.write_str("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
  out.write_fmt("<testsuites tests=\"{}\" name=\"AllTests\">\n", all_cnt);
  for (auto& suite : suites) {
    const auto tests = suite.tests();
    out.write_fmt("  <testsuite name=\"{}\" tests=\"{}\">\n", suite.name(), tests.len());
    for (auto& test : tests) {
      out.write_fmt("    <testcase name=\"{}\" file=\"{}\" line=\"{}\"/>\n",
                    test.name(),
                    Str{test._loc.file},
                    test._loc.line);
    }
    out.write_str("  </testsuite>\n");
  }
  out.write_str("</testsuites>\n");

  return buf;
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

void App::exec(Str filter, Option<bool> color_opt) {
  auto& all_test = AllTest::instance();

  const auto pats = parse_filter(filter);
  const auto color = color_opt ? *color_opt : io::Stdout::is_tty();

  all_test.run(pats.as_slice(), color);
}

void App::list() const {
  auto& all_test = AllTest::instance();

  auto outs = io::Stdout::lock();
  for (auto& suite : all_test.suites()) {
    outs.write_fmt("{}\n", suite.name());
    for (auto& test : suite.tests()) {
      outs.write_fmt("    {}\n", test.name());
    }
  }
}

void App::list_xml(Str path) const {
  auto& all_test = AllTest::instance();

  const auto xml_str = format_xml(all_test.suites());
  if (path.is_empty() || path == "stdout") {
    io::println(xml_str.as_str());
    return;
  }

  const auto pos = path.find(':');
  const auto xml_path = path[{pos ? (*pos + 1U) : 0, $}];
  auto xml_file = fs::File::create(fs::Path{xml_path});
  if (xml_file.is_err()) {
    io::println("Failed to create file '{}'", xml_path);
    return;
  }
  (void)xml_file->write_str(xml_str.as_str());
}

auto main(int argc, const char* argv[]) -> int {
  auto cmd = app::Clap{"sfc_test"};
  cmd.add_flag("h:help", "Show this help message");
  cmd.add_flag("l:list", "List all tests");
  cmd.add_flag("gtest_list_tests", "List all tests instead of running them");
  cmd.add_opt("gtest_filter", "Run only tests matching the given pattern", "PATTERNS");
  cmd.add_opt("gtest_color", "Enable or disable colored output", "BOOL");
  cmd.add_opt("gtest_output", "Output file for test results", "FILE");

  if (!cmd.parse_cmdline(argc, argv)) {
    cmd.print_help();
    return -1;
  }

  auto app = App{};
  if (cmd.get("help")) {
    app.help();
    return 0;
  }

  if (cmd.get("list")) {
    app.list();
    return 0;
  }

  if (cmd.get("gtest_list_tests")) {
    const auto output = cmd.get("gtest_output").unwrap_or("stdout");
    app.list_xml(output);
    return 0;
  }

  const auto is_tty = io::Stdout::is_tty();
  const auto gtest_color = cmd.get_flag("gtest_color").unwrap_or(is_tty);
  const auto gtest_filter = cmd.get("gtest_filter").unwrap_or("");
  app.exec(gtest_filter, gtest_color);
  return 0;
}

}  // namespace sfc::test
