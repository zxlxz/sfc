#include "rc/test.h"

#include "rc/core/fmt.h"

namespace rc::fmt {

rc_test(fmt_int) {
  rc::assert_eq(string::format(u8"{}", 123), u8"123");

  rc::assert_eq(string::format(u8"{5}", 123), u8"  123");
  rc::assert_eq(string::format(u8"{<5}", 123), u8"123  ");
  rc::assert_eq(string::format(u8"{>5}", 123), u8"  123");
  rc::assert_eq(string::format(u8"{^5}", 123), u8" 123 ");

  rc::assert_eq(string::format(u8"{5}", -12), u8"  -12");
  rc::assert_eq(string::format(u8"{<5}", -12), u8"-12  ");
  rc::assert_eq(string::format(u8"{>5}", -12), u8"  -12");
  rc::assert_eq(string::format(u8"{^5}", -12), u8" -12 ");
  rc::assert_eq(string::format(u8"{=5}", -12), u8"-  12");
}

rc_test(fmt_flt) {
  rc::assert_eq(string::format(u8"{.1}", 1.2), u8"1.2");
  rc::assert_eq(string::format(u8"{5.1}", 1.2), u8"  1.2");
  rc::assert_eq(string::format(u8"{<5.1}", 1.2), u8"1.2  ");
  rc::assert_eq(string::format(u8"{>5.1}", 1.2), u8"  1.2");
  rc::assert_eq(string::format(u8"{^5.1}", 1.2), u8" 1.2 ");
}
}  // namespace rc::fmt
