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
  static constexpr usize DEFAULT_BUFF_SIZE = 1024U;
  using Buf = Vec<u8>;

  R _inn;
  Buf _buf = Buf::with_capacity(DEFAULT_BUFF_SIZE);
  usize _pos = 0;

 public:
  explicit BufReader(R&& inn) noexcept : _inn{static_cast<R&&>(inn)} {}
  ~BufReader() noexcept = default;

  BufReader(BufReader&&) noexcept = default;
  BufReader& operator=(BufReader&&) noexcept = default;

 public:
  auto buffer() const -> Slice<const u8> {
    return _buf[{_pos, $}];
  }

  auto capacity() const -> usize {
    return _buf.capacity();
  }

  auto peak(usize n) -> Result<Slice<const u8>> {
    if (n > _buf.len() - _pos) {
      if (_pos + n > _buf.capacity()) {
        this->backshift();
      }
      _TRY(this->read_more());
    }
    return static_cast<const Buf&>(_buf)[{_pos, _pos + n}];
  }

  void backshift() {
    if (_pos == 0) {
      return;
    }
    _buf.drain({0, _pos});
    _pos = 0;
  }

  void discard_buffer() {
    _buf.set_len(0);
    _pos = 0;
  }

  auto read_more() -> Result<usize> {
    const auto cnt = _TRY(_inn.read(_buf.spare_capacity_mut()));
    _buf.set_len(_buf.len() + cnt);
    return cnt;
  }

 public:
  // trait:: io::Read
  auto read(Slice<u8> buf) -> Result<usize> {
    // if we dont't have any buffered data
    // read directly into the user's buffer
    if (_pos >= _buf.len() && buf.len() >= _buf.capacity()) {
      this->discard_buffer();
      return _inn.read(buf);
    }
    auto rem = _TRY(this->fill_buf());
    const auto nread = rem.read(buf).unwrap_or(0);
    this->consume(nread);
    return nread;
  }

 public:
  // trait: io::BufRead
  auto fill_buf() -> Result<Slice<const u8>> {
    if (_pos >= _buf.len()) {
      this->backshift();
      _TRY(this->read_more());
    }
    return this->buffer();
  }

  // trait: io::BufRead
  void consume(usize amt) {
    if (amt > _buf.len() - _pos) {
      amt = _buf.len() - _pos;
    }
    _pos += amt;
  }

  auto skip(auto&& p) -> Result<usize> {
    auto res = usize{0UL};
    while (true) {
      const auto rem = _TRY(this->fill_buf());
      const auto pos = rem.iter().position([&](auto c) { return !p(c); });
      const auto cnt = pos ? *pos : rem.len();
      res += cnt;
      this->consume(cnt);
      if (pos) {
        break;
      }
    }
    return res;
  }

  auto read_until(u8 delim, Vec<u8>& buf) -> Result<usize> {
    const auto old_len = buf.len();
    while (true) {
      const auto available = _TRY(this->fill_buf());
      const auto position = available.find(delim);
      const auto used = position ? *position + 1 : available.len();
      buf.extend_from_slice(available[{0, used}]);
      this->consume(used);
      if (position || used == 0) {
        break;
      }
    }
    return buf.len() - old_len;
  }
};

}  // namespace sfc::io
