#include "rc.inl"

#include "rc/term/mod.h"

#include "rc/sys.h"

namespace rc::term {

pub auto columns() -> usize {
  const auto res = sys::term::columns();
  return res;
}

}  // namespace rc::term
