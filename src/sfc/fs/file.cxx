#include "sfc/test/test.h"
#include "sfc/fs.h"
#include "sfc/env.h"

namespace sfc::fs::test {

SFC_TEST(read_write) {
  const auto path_buf = env::temp_dir().join(Path{"test_file.txt"});

  // create, write
  {
    auto file_res = File::create(path_buf.as_path());
    sfc::assert_eq(file_res.is_ok(), true);
    auto file = mem::move(file_res).unwrap();
    (void)file.write_str("hello world");
  }

  // open, read
  {
    auto file_res = File::open(path_buf.as_path());
    sfc::assert_eq(file_res.is_ok(), true);
    auto file = mem::move(file_res).unwrap();

    String read_content;
    auto read_res = file.read_to_string(read_content);
    sfc::assert_eq(read_res.is_ok(), true);
    sfc::assert_eq(read_content.as_str(), Str{"hello world"});
  }
}

}  // namespace sfc::fs::test
