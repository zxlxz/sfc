#include "sfc/fs.h"
#include "sfc/test.h"

namespace sfc::fs {

SFC_TEST(path_file_name) {
  test::assert_eq(Path{"/usr/bin"}.file_name(), "bin");
  test::assert_eq(Path{"tmp/foo.txt"}.file_name(), "foo.txt");
  test::assert_eq(Path{"foo.txt/."}.file_name(), "foo.txt");
  test::assert_eq(Path{"foo.txt/.//"}.file_name(), "foo.txt");

  test::assert_eq(Path{"foo.txt/.."}.file_name(), "");
  test::assert_eq(Path{"/"}.file_name(), "");
}

SFC_TEST(path_file_stem) {
  test::assert_eq(Path{"foo.cc"}.file_stem(), "foo");
  test::assert_eq(Path{"foo.tar.gz"}.file_stem(), "foo.tar");
}

SFC_TEST(path_extension) {
  test::assert_eq(Path{"foo.cc"}.extension(), "cc");
  test::assert_eq(Path{"foo.tar.gz"}.extension(), "gz");
}

SFC_TEST(path_parrent) {
  test::assert_eq(Path{"/foo/bar"}.parent().as_str(), "/foo");
  test::assert_eq(Path{"/"}.parent().as_str(), "");
}

}  // namespace sfc::fs
