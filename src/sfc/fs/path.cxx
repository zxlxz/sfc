#include "sfc/fs.h"
#include "sfc/test/test.h"

namespace sfc::fs::path::test {

SFC_TEST(file_name) {
  panicking::expect_eq(Path{"/usr/bin"}.file_name(), "bin");
  panicking::expect_eq(Path{"tmp/foo.txt"}.file_name(), "foo.txt");
  panicking::expect_eq(Path{"foo.txt/."}.file_name(), "foo.txt");
  panicking::expect_eq(Path{"foo.txt/.//"}.file_name(), "foo.txt");

  panicking::expect_eq(Path{"foo.txt/.."}.file_name(), "");
  panicking::expect_eq(Path{"/"}.file_name(), "");
}

SFC_TEST(file_stem) {
  panicking::expect_eq(Path{"foo.cc"}.file_stem(), "foo");
  panicking::expect_eq(Path{"foo.tar.gz"}.file_stem(), "foo.tar");
}

SFC_TEST(extension) {
  panicking::expect_eq(Path{"foo.cc"}.extension(), "cc");
  panicking::expect_eq(Path{"foo.tar.gz"}.extension(), "gz");
}

SFC_TEST(parrent) {
  panicking::expect_eq(Path{"/foo/bar"}.parent().as_str(), "/foo");
  panicking::expect_eq(Path{"/"}.parent().as_str(), "");
}

SFC_TEST(is_absolute) {
  panicking::expect_true(Path{"/usr/bin"}.is_absolute());
  panicking::expect_false(Path{"tmp/foo.txt"}.is_absolute());
  panicking::expect_false(Path{"foo.txt/."}.is_absolute());
  panicking::expect_false(Path{"foo.txt/.//"}.is_absolute());
}

SFC_TEST(is_relative) {
  panicking::expect_false(Path{"/usr/bin"}.is_relative());
  panicking::expect_true(Path{"tmp/foo.txt"}.is_relative());
  panicking::expect_true(Path{"foo.txt/."}.is_relative());
  panicking::expect_true(Path{"foo.txt/.//"}.is_relative());
}

SFC_TEST(push) {
  auto p = PathBuf::from("tmp");
  p.push(Path{"foo.txt"});
  panicking::expect_eq(p.as_str(), "tmp/foo.txt");
}

SFC_TEST(pop) {
  auto p = PathBuf::from("tmp/foo.txt");

  p.pop();
  panicking::expect_eq(p.as_str(), "tmp");
}

SFC_TEST(join) {
  const auto p = Path{"tmp"};
  panicking::expect_eq(p.join("foo.txt").as_str(), "tmp/foo.txt");
}

SFC_TEST(set_file_name) {
  auto p = PathBuf::from("tmp/foo.txt");
  p.set_file_name("bar.txt");
  panicking::expect_eq(p.as_str(), "tmp/bar.txt");
}

SFC_TEST(set_extension) {
  auto p = PathBuf::from("tmp/foo.txt");
  p.set_extension("md");
  panicking::expect_eq(p.as_str(), "tmp/foo.md");
}

}  // namespace sfc::fs::path::test
