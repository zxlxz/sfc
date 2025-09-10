#pragma once

#include "sfc/io/mod.h"

namespace sfc::io {

struct BufRead {
  Slice<const u8> _inn;

 public:
  auto read(Slice<u8> buf) -> Result<usize> {
    const auto amt = _inn._len < buf._len ? _inn._len : buf._len;
    const auto [a, b] = _inn.split_at(amt);
    buf[{0, amt}].copy_from_slice(a);
    _inn = b;
    return amt;
  }

  auto read_to_end(Vec<u8>& buf) -> Result<usize> {
    const auto amt = _inn._len;
    buf.extend_from_slice(_inn);
    _inn = {};
    return amt;
  }

  auto read_to_string(String& buf) -> Result<usize> {
    const auto amt = _inn._len;
    buf.write_str(Str::from_u8(_inn));
    _inn = {};
    return amt;
  }
};

struct BufWrite {
  Vec<u8>& _inn;

 public:
  auto write(Slice<const u8> buf) -> Result<usize> {
    _inn.extend_from_slice(buf);
    return buf._len;
  }

  auto write_all(Slice<const u8> buf) -> Result<usize> {
    _inn.extend_from_slice(buf);
    return buf._len;
  }

  auto write_str(Str buf) -> Result<usize> {
    _inn.extend_from_slice(buf.as_bytes());
    return buf._len;
  }
};

}  // namespace sfc::io
