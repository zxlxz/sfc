#include "rc.inl"

#include "rc/sys.h"

namespace rc::panicking {

pub auto panic_str(Str s) -> void {
  // redirect to system::panic
  sys::panicking::panic(s);
}

}  // namespace rc::panicking
