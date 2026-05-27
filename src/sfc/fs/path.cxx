#include "sfc/fs.h"
#include "sfc/test/test.h"

namespace sfc::fs::path::test {

SFC_TEST(file_name) {
  sfc::assert_eq(Path{"/usr/bin"}.file_name(), "bin");
  sfc::assert_eq(Path{"tmp/foo.txt"}.file_name(), "foo.txt");

  sfc::assert_eq(Path{"foo.txt/."}.file_name(), "");
  sfc::assert_eq(Path{"foo.txt/.."}.file_name(), "");
  sfc::assert_eq(Path{"foo.txt/.//"}.file_name(), "");
  sfc::assert_eq(Path{"/"}.file_name(), "");
}

SFC_TEST(file_stem) {
  sfc::assert_eq(Path{"foo.cc"}.file_stem(), "foo");
  sfc::assert_eq(Path{"foo.tar.gz"}.file_stem(), "foo.tar");
}

SFC_TEST(extension) {
  sfc::assert_eq(Path{"foo.cc"}.extension(), "cc");
  sfc::assert_eq(Path{"foo.tar.gz"}.extension(), "gz");
}

SFC_TEST(parrent) {
  sfc::assert_eq(Path{"/"}.parent(), Path{""});
  sfc::assert_eq(Path{"/foo"}.parent(), Path{"/"});
  sfc::assert_eq(Path{"/foo/"}.parent(), Path{"/"});
  sfc::assert_eq(Path{"/foo/bar"}.parent(), Path{"/foo"});
  sfc::assert_eq(Path{"/foo/bar/"}.parent(), Path{"/foo"});
}

SFC_TEST(is_absolute) {
  sfc::assert_eq(Path{"/usr/bin"}.is_absolute(), true);

  sfc::assert_eq(Path{"tmp/foo.txt"}.is_absolute(), false);
  sfc::assert_eq(Path{"foo.txt/."}.is_absolute(), false);
  sfc::assert_eq(Path{"foo.txt/.//"}.is_absolute(), false);
}

SFC_TEST(is_relative) {
  sfc::assert_eq(Path{"/usr/bin"}.is_relative(), false);

  sfc::assert_eq(Path{"tmp/foo.txt"}.is_relative(), true);
  sfc::assert_eq(Path{"foo.txt/."}.is_relative(), true);
  sfc::assert_eq(Path{"foo.txt/.//"}.is_relative(), true);
}

SFC_TEST(push) {
  auto p = PathBuf::from("tmp");
  p.push(Path{"foo.txt"});
  sfc::assert_eq(p.as_str(), "tmp/foo.txt");
}

SFC_TEST(pop) {
  auto p = PathBuf::from("tmp/foo.txt");

  p.pop();
  sfc::assert_eq(p.as_str(), "tmp");
}

SFC_TEST(join) {
  const auto p = Path{"tmp"};
  sfc::assert_eq(p.join("foo.txt").as_str(), "tmp/foo.txt");
}

SFC_TEST(set_file_name) {
  auto p = PathBuf::from("tmp/foo.txt");
  p.set_file_name("bar.txt");
  sfc::assert_eq(p.as_str(), "tmp/bar.txt");
}

SFC_TEST(set_extension) {
  auto p = PathBuf::from("tmp/foo.txt");
  p.set_extension("md");
  sfc::assert_eq(p.as_str(), "tmp/foo.md");
}

}  // namespace sfc::fs::path::test
