#pragma once

#include "rc/io/mod.h"

namespace rc::io {

template <class Self>
struct Read {
  Self& _self;

  auto read(Slice<u8> buf) -> usize {
    const auto ret = _self.read(buf);
    return ret;
  }

  auto read_exact(Slice<u8> buf) -> void {
    while (!buf.is_empty()) {
      const auto ret = _self.read(buf);
      if (ret == 0) {
        rc::panic(u8"read to eof");
      }
      buf._ptr += ret;
      buf._len -= ret;
    }
  }

  auto read_to_end(Vec<u8>& buf, usize buf_size = 64) noexcept -> usize {
    const auto old_len = buf.len();
    while (true) {
      buf.reserve(buf_size);
      const auto rbuf = Slice{buf.as_mut_ptr() + buf.len(), buf_size};
      const auto nret = _self.read(rbuf);
      if (nret == 0) {
        break;
      }
      buf._len += nret;
    }
    const auto ret = buf._len - old_len;
    return ret;
  }

  auto read_to_string() noexcept -> String {
    auto ret = String();
    this->read_to_end(ret._vec);
    return ret;
  }
};

template <class Self>
Read(Self&)->Read<Self>;

}  // namespace rc::io
