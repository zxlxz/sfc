#pragma once

#include "rc/core/fmt.h"

namespace rc::panicking {

enum Error {};

struct Location {
  Str _file;
  Str _func;
  u32 _line;
};

pub auto panic_str(Str s) -> void;

template<class ...T>
auto panic_fmt(fmt::Args<T...> f) -> void {
  u8 raw[1024];
  auto sbuf = fmt::Buffer::from_array(raw);
  auto sfmt = fmt::Formatter{sbuf};
  sfmt.write_fmt(f);
  panicking::panic_str(sbuf.as_str());
}

}  // namespace rc::panicking
