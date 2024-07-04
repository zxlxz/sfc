#include "sfc/fs.h"
#include "sfc/test.h"

namespace sfc::fs::path::test {

SFC_TEST(file_name) {
  panicking::assert_eq(Path{"/usr/bin"}.file_name(), "bin");
  panicking::assert_eq(Path{"tmp/foo.txt"}.file_name(), "foo.txt");
  panicking::assert_eq(Path{"foo.txt/."}.file_name(), "foo.txt");
  panicking::assert_eq(Path{"foo.txt/.//"}.file_name(), "foo.txt");

  panicking::assert_eq(Path{"foo.txt/.."}.file_name(), "");
  panicking::assert_eq(Path{"/"}.file_name(), "");
}

SFC_TEST(file_stem) {
  panicking::assert_eq(Path{"foo.cc"}.file_stem(), "foo");
  panicking::assert_eq(Path{"foo.tar.gz"}.file_stem(), "foo.tar");
}

SFC_TEST(extension) {
  panicking::assert_eq(Path{"foo.cc"}.extension(), "cc");
  panicking::assert_eq(Path{"foo.tar.gz"}.extension(), "gz");
}

SFC_TEST(parrent) {
  panicking::assert_eq(Path{"/foo/bar"}.parent().as_str(), "/foo");
  panicking::assert_eq(Path{"/"}.parent().as_str(), "");
}

SFC_TEST(is_absolute) {
  panicking::assert_true(Path{"/usr/bin"}.is_absolute());
  panicking::assert_false(Path{"tmp/foo.txt"}.is_absolute());
  panicking::assert_false(Path{"foo.txt/."}.is_absolute());
  panicking::assert_false(Path{"foo.txt/.//"}.is_absolute());
}

SFC_TEST(is_relative) {
  panicking::assert_false(Path{"/usr/bin"}.is_relative());
  panicking::assert_true(Path{"tmp/foo.txt"}.is_relative());
  panicking::assert_true(Path{"foo.txt/."}.is_relative());
  panicking::assert_true(Path{"foo.txt/.//"}.is_relative());
}

SFC_TEST(push) {
  auto p = PathBuf::from("tmp");
  p.push("foo.txt");
  panicking::assert_eq(p.as_path(), Path{"tmp/foo.txt"});
}

SFC_TEST(pop) {
  auto p = PathBuf::from("tmp/foo.txt");

  p.pop();
  panicking::assert_eq(p.as_path(), Path{"tmp"});
}

SFC_TEST(join) {
  const auto p = Path{"tmp"};
  panicking::assert_eq(p.join("foo.txt").as_path(), Path{"tmp/foo.txt"});
}

SFC_TEST(set_file_name) {
  auto p = PathBuf::from("tmp/foo.txt");
  p.set_file_name("bar.txt");
  panicking::assert_eq(p.as_path(), Path{"tmp/bar.txt"});
}

SFC_TEST(set_extension) {
  auto p = PathBuf::from("tmp/foo.txt");
  p.set_extension("md");
  panicking::assert_eq(p.as_path(), Path{"tmp/foo.md"});
}

}  // namespace sfc::fs::path::test
