#include "rc.inl"

#include "rc/sys.h"

namespace rc::os {

pub auto last_error() -> i32 {
  const auto res = sys::os::last_error();
  return res;
}

}  // namespace rc::os

