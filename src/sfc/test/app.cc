#include "sfc/fs.h"
#include "sfc/io.h"
#include "sfc/serde/xml.h"
#include "sfc/test.h"
#include "sfc/test/app.h"

namespace sfc::test {

static auto parse_filter(Str filter) -> Vec<Str> {
  auto find_pos = [](Str s) -> usize {
    for (auto p = 0U; p < s._len; p++) {
      switch (s[p]) {
        case ';':
          return p;
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
  while (filter) {
    const auto pos = find_pos(filter);
    v.push(filter[{0, pos}]);
    filter = filter[{pos + 1, $}];
  }
  return v;
}

static auto format_xml(Slice<const Suite> suites) -> String {
  auto buf = String{};
  auto ser = serde::xml::Serializer{buf};

  auto all_cnt = usize{0};
  for (const auto& suite : suites) {
    all_cnt += suite.tests().len();
  }

  auto testsuites = ser.serialize_node("testsuites");
  testsuites.attr("tests", all_cnt);
  testsuites.attr("name", "AllTests");

  for (auto& suite : suites) {
    auto testsuite = testsuites.serialize_node("testsuite");
    testsuite.attr("name", suite.name());
    testsuite.attr("tests", suite.tests().len());
    for (auto& test : suite.tests()) {
      auto testcase = testsuite.serialize_node("testcase");
      testcase.attr("name", test.name());
      testcase.attr("file", Str::from_cstr(test._loc.file));
      testcase.attr("line", test._loc.line);
    }
  }

  return buf;
}

auto suites() -> Slice<Suite>;

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
  const auto pats = parse_filter(filter);
  const auto color = color_opt ? *color_opt : io::Stdout::is_tty();

  auto suites = test::suites();
  for (auto& suite : suites) {
    suite.run(pats.as_slice(), color);
  }
}

void App::list() const {
  auto suites = test::suites();

  auto outs = io::Stdout::lock();
  for (auto& suite : suites) {
    outs.write_fmt("{}\n", suite.name());
    for (auto& test : suite.tests()) {
      outs.write_fmt("    {}\n", test.name());
    }
  }
}

void App::list_xml(Str path) const {
  auto suites = test::suites();
  const auto xml_str = format_xml(suites);
  if (!path || path == "stdout") {
    io::println(xml_str);
    return;
  }

  const auto pos = path.find(':');
  const auto xml_path = pos ? path[{*pos + 1, $}] : path;
  auto xml_file = fs::File::create(fs::Path{xml_path}).ok();
  if (!xml_file) {
    io::println("Failed to create file '{}'", xml_path);
    return;
  }
  xml_file->write_str(xml_str);
}

void App::main(int argc, const char* argv[]) {
  auto cmd = app::Cmd{"sfc_test"};
  cmd.add_opt("h:help", "Show this help message");
  cmd.add_opt("l:list", "List all tests");
  cmd.add_opt("gtest_list_tests", "List all tests instead of running them");
  cmd.add_opt("gtest_filter", "Run only tests matching the given pattern");
  cmd.add_opt("gtest_color", "Enable or disable colored output");
  cmd.add_opt("gtest_output", "Output file for test results");

  cmd.parse_cmdline(argc, argv);

  if (cmd.get("help")) {
    this->help();
    return;
  }

  if (cmd.get("list")) {
    this->list();
    return;
  }

  if (cmd.get("gtest_list_tests")) {
    const auto output = cmd.get("gtest_output").unwrap_or("stdout");
    this->list_xml(output);
    return;
  }

  const auto gtest_color = cmd.get_flag("gtest_color");
  const auto gtest_filter = cmd.get("gtest_filter").unwrap_or("");
  this->exec(gtest_filter, gtest_color);
}

}  // namespace sfc::test
