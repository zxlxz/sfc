#include "sfc/time.h"

#include "sfc/io.h"
#include "sfc/test.h"

namespace sfc::time {

SFC_TEST(date_time) {
  auto t = DateTime::now();
  io::println("time.now = `{}`", t);
}

}  // namespace sfc::time
