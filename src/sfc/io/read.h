#pragma once

#include "sfc/io/mod.h"
#include "sfc/alloc/vec.h"
#include "sfc/alloc/string.h"

namespace sfc::io {

struct Read {
  auto read_exact(this auto& self, Slice<u8> buf) -> Result<> {
    while (!buf.is_empty()) {
      const auto cnt = _TRY(Result{self.read(buf)});
      if (cnt == 0) {
        return Error{ErrorKind::UnexpectedEof, 0};
      }
      buf = buf[{cnt, $}];
    }

    return {};
  }

  auto read_to_end(this auto& self, Vec<u8>& buf, usize buf_len = 256) -> Result<usize> {
    const auto old_len = buf.len();
    while (true) {
      buf.reserve(buf_len);

      auto tmp = Slice{buf.as_mut_ptr() + buf.len(), buf_len};
      const auto cnt = _TRY(Result{self.read(tmp)});
      if (cnt == 0) {
        break;
      }
      buf.set_len(buf.len() + cnt);
    }
    return buf.len() - old_len;
  }

  auto read_to_string(this auto& self, String& buf) -> Result<usize> {
    return self.read_to_end(buf.as_mut_vec());
  }
};

template <class R>
class BufReader : Read {
  static constexpr usize BUFF_SIZE = 4096U;

  R _inn;
  Vec<u8> _buf = {};
  usize _pos = 0;

 public:
  explicit BufReader(R&& inn) noexcept : _inn{static_cast<R&&>(inn)}, _buf{Vec<u8>::with_capacity(BUFF_SIZE)} {}
  ~BufReader() noexcept = default;

  BufReader(BufReader&&) noexcept = default;
  BufReader& operator=(BufReader&&) noexcept = default;

 public:
  auto fill_buf() -> Result<Slice<const u8>> {
    if (_pos == _buf.len()) {
      const auto cnt = _TRY(Result{_inn.read(_buf.spare_capacity_mut())});
      _pos = 0;
      _buf.set_len(cnt);
    }
    return Slice<const u8>{_buf[{_pos, _buf.len()}]};
  }

  void consume(usize amt) {
    _pos = num::min(_pos + amt, _buf.len());
  }

  auto peak(usize n) -> Result<Slice<const u8>> {
    if (_buf.len() - _pos < n) {
      _buf.drain({0, _pos});
      _pos = 0;

      const auto cnt = _TRY(Result{_inn.read(_buf.spare_capacity_mut())});
      _buf.set_len(_buf.len() + cnt);
    }
    return Slice<const u8>{_buf[{_pos, _pos + n}]};
  }

  auto read(Slice<u8> buf) -> Result<usize> {
    if (_buf.is_empty() && buf.len() > _buf.capacity()) {
      return _inn.read(buf);
    }

    const auto rem = this->fill_buf();
    const auto cnt = rem.read(buf);
    this->consume(cnt);
    return cnt;
  }

  auto skip(auto&& p) -> Result<usize> {
    auto res = usize{0UL};
    while (true) {
      const auto rem = _TRY(this->fill_buf());
      const auto cnt = rem.iter().position([&](auto c) { return !p(c); }).unwrap_or(rem.len());
      this->consume(cnt);
      res += cnt;
      if (cnt != rem.len()) {
        break;
      }
    }
    return res;
  }

  auto read_until(auto&& p, Vec<u8>& buf) -> Result<usize> {
    const auto old_len = buf.len();
    while (true) {
      const auto rem = _TRY(this->fill_buf());
      const auto pos = rem.iter().position(p);
      const auto cnt = pos.unwrap_or(rem.len() - 1) + 1;
      buf.extend_from_slice(rem.slice(0, cnt));
      this->consume(cnt);
      if (pos) {
        break;
      }
    }
    return buf.len() - old_len;
  }
};

}  // namespace sfc::io
