#include "sfc/fs.h"
#include "sfc/test.h"

namespace sfc::fs {

SFC_TEST(path_file_name) {
  panicking::assert_eq(Path::from("/usr/bin").file_name(), "bin");
  panicking::assert_eq(Path::from("tmp/foo.txt").file_name(), "foo.txt");
  panicking::assert_eq(Path::from("foo.txt/.").file_name(), "foo.txt");
  panicking::assert_eq(Path::from("foo.txt/.//").file_name(), "foo.txt");

  panicking::assert_eq(Path::from("foo.txt/..").file_name(), "");
  panicking::assert_eq(Path::from("/").file_name(), "");
}

SFC_TEST(path_file_stem) {
  panicking::assert_eq(Path::from("foo.cc").file_stem(), "foo");
  panicking::assert_eq(Path::from("foo.tar.gz").file_stem(), "foo.tar");
}

SFC_TEST(path_extension) {
  panicking::assert_eq(Path::from("foo.cc").extension(), "cc");
  panicking::assert_eq(Path::from("foo.tar.gz").extension(), "gz");
}

SFC_TEST(path_parrent) {
  panicking::assert_eq(Path::from("/foo/bar").parent().as_str(), "/foo");
  panicking::assert_eq(Path::from("/").parent().as_str(), "");
}

}  // namespace sfc::fs
