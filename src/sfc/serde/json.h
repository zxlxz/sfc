#pragma once

#include "sfc/serde/mod.h"

namespace sfc::serde::json {

template <class W>
class Serializer {
  W& _write;

 public:
  explicit Serializer(W& write) noexcept : _write{write} {}
  ~Serializer() noexcept = default;
  Serializer(const Serializer&) noexcept = delete;

 public:
  void serialize_null() {
    _write.write_str("null");
  }

  void serialize_bool(bool val) {
    _write.write_str(val ? Str{"true"} : Str{"false"});
  }

  void serialize_char(char val) {
    const char s[] = {'"', val, '"'};
    _write.write_str({s, 3});
  }

  void serialize_int(auto val) {
    auto f = fmt::Fmter{_write};
    fmt::Display::fmt(val, f);
  }

  void serialize_flt(auto val) {
    auto f = fmt::Fmter{_write};
    fmt::Display::fmt(val, f);
  }

  void serialize_str(Str val) {
    _write.write_char('"');
    _write.write_str(val);
    _write.write_char('"');
  }

  auto serialize_bin(Slice<const u8> val) = delete;

  struct SerSeq;
  auto serialize_seq() -> SerSeq {
    return SerSeq{*this};
  }

  struct SerMap;
  auto serialize_map() -> SerMap {
    return SerMap{*this};
  }
};

template <class W>
class Serializer<W>::SerSeq {
  Serializer& _inn;
  u32 _cnt = 0;

 public:
  explicit SerSeq(Serializer& ser) noexcept : _inn{ser} {
    _inn._write.write_char('[');
  }

  ~SerSeq() noexcept {
    _inn._write.write_char(']');
  }

  SerSeq(const SerSeq&) noexcept = delete;

  void serialize_element(const auto& item) {
    if (_cnt++ != 0) {
      _inn._write.write_char(',');
    }
    Serialize::serialize(item, _inn);
  }
};

template <class W>
class Serializer<W>::SerMap {
  Serializer& _inn;
  u32 _cnt = 0;

 public:
  explicit SerMap(Serializer& ser) noexcept : _inn{ser} {
    _inn._write.write_char('{');
  }

  ~SerMap() noexcept {
    _inn._write.write_char('}');
  }

  SerMap(const SerMap&) noexcept = delete;

  void serialize_entry(str::Str key, const auto& value) {
    if (_cnt++ != 0) {
      _inn._write.write_char(',');
    }
    _inn._write.write_char('"');
    _inn._write.write_str(key);
    _inn._write.write_char('"');
    _inn._write.write_char(':');
    Serialize::serialize(value, _inn);
  }
};

template <class R>
class Deserializer {
  static_assert(!trait::same_<R, const R>);
  using Error = io::Error;

  io::BufReader<R&> _read;

 public:
  explicit Deserializer(R& read) noexcept : _read{read} {}
  ~Deserializer() noexcept = default;
  Deserializer(const Deserializer&) noexcept = delete;

 public:
  auto deserialize_null() -> io::Result<> {
    const auto c = _TRY(this->peak());
    if (c != 'n') {
      return io::Error{io::ErrorKind::InvalidData};
    }
    return this->pop_match("null");
  }

  auto deserialize_bool() -> io::Result<bool> {
    const auto c = _TRY(this->peak());
    if (c == 't' && this->pop_match("true").is_ok()) {
      return true;
    }
    if (c == 'f' && this->pop_match("false").is_ok()) {
      return false;
    }
    return io::Error{io::ErrorKind::InvalidData};
  }

  template <class T>
  auto deserialize_int() -> io::Result<T> {
    _TRY(this->peak());
    const auto s = _TRY(this->pop_num());
    const auto n = s.template parse<T>();
    if (!n) {
      return io::Error{io::ErrorKind::InvalidData};
    }
    return n.unwrap();
  }

  template <class T>
  auto deserialize_flt() -> io::Result<T> {
    _TRY(this->peak());
    const auto s = _TRY(this->pop_num());
    const auto n = s.template parse<T>();
    if (!n) {
      return io::Error{io::ErrorKind::InvalidData};
    }
    return n.unwrap();
  }

  auto deserialize_str() -> io::Result<Str> {
    const auto c = _TRY(this->peak());
    if (c != '"') {
      return io::Error{io::ErrorKind::InvalidData};
    }
    return this->pop_str();
  }

  class DesSeq;
  auto deserialize_seq() -> DesSeq {
    return DesSeq{*this};
  }

  class DesMap;
  auto deserialize_map() -> DesMap {
    return DesMap{*this};
  }

 private:
  auto peak() -> io::Result<char> {
    static const auto is_blank = +[](char c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); };
    _TRY(_read.skip(is_blank));

    const auto b = _TRY(_read.peak(1));
    if (b.is_empty()) {
      return io::Error{io::ErrorKind::UnexpectedEof};
    }
    return char(b[0]);
  }

  auto pop_match(Str s) -> io::Result<> {
    const auto b = _TRY(_read.peak(s.len()));
    if (b != s.as_bytes()) {
      return io::Error{io::ErrorKind::InvalidData};
    }
    return {};
  }

  auto pop_num() -> io::Result<Str> {
    static const auto not_digits = +[](char c) { return !('0' <= c && c <= '9' || c == '.' || c == '+' || c == '-'); };

    const auto b = _TRY(_read.peak(32));
    const auto n = b.iter().position(not_digits).unwrap_or(b.len());
    if (n == 0) {
      return io::Error{io::ErrorKind::InvalidData};
    }
    _read.consume(n);
    return Str::from_u8(b[{0, n}]);
  }

  auto pop_str() -> io::Result<Str> {
    _read.consume(1);

    auto buf = _TRY(_read.fill_buf());
    auto pos = buf.find('"');
    if (!pos) {
      buf = _TRY(_read.peak(buf.len() + 2048));
      pos = buf.find('"');
    }
    if (!pos) {
      return io::Error{io::ErrorKind::InvalidData};
    }
    const auto n = pos.unwrap();
    _read.consume(n + 1);
    return Str::from_u8(buf[{0, n}]);
  }
};

template <class R>
class Deserializer<R>::DesSeq {
  Deserializer& _inn;
  usize _count = 0;
  bool _ended = false;

 public:
  explicit DesSeq(Deserializer& des) noexcept : _inn{des} {}
  ~DesSeq() noexcept = default;
  DesSeq(const DesSeq&) noexcept = delete;

  struct Item {
    Deserializer& _inn;
    auto visit(auto& val) -> io::Result<> {
      return Deserialize::deserialize(val, _inn);
    }
  };

  auto next() -> io::Result<Option<Item>> {
    if (_ended) {
      return io::Error{io::ErrorKind::InvalidData};
    }

    if (_count == 0) {
      const auto c = _TRY(_inn.peak());
      if (c != '[') {
        return io::Error{io::ErrorKind::InvalidData};
      }
      _inn._read.consume(1);
    }

    const auto c = _TRY(_inn.peak());
    if (c == ']') {
      _ended = true;
      return Option<Item>{};
    }

    if (_count++ != 0) {
      if (c != ',') {
        return io::Error{io::ErrorKind::InvalidData};
      }
      _inn._read.consume(1);
    }

    return Option<Item>{{_inn}};
  }
};

template <class R>
class Deserializer<R>::DesMap {
  Deserializer& _inn;
  usize _count = 0;
  bool _ended = false;

 public:
  explicit DesMap(Deserializer& des) noexcept : _inn{des} {}
  ~DesMap() noexcept = default;
  DesMap(const DesMap&) noexcept = delete;

  struct Item {
    Deserializer& _inn;

    auto visit_key(auto& key) -> io::Result<> {
      const auto k = _TRY(_inn.deserialize_str());
      key = k;
      return {};
    }

    auto visit_val(auto& val) -> io::Result<> {
      const auto c = _TRY(_inn.peak());
      if (c != ':') {
        return io::Error{io::ErrorKind::InvalidData};
      }
      _inn._read.consume(1);
      return Deserialize::deserialize(val, _inn);
    }
  };

  auto next() -> io::Result<Option<Item>> {
    if (_ended) {
      return io::Error{io::ErrorKind::InvalidData};
    }

    if (_count == 0) {
      const auto c = _TRY(_inn.peak());
      if (c != '{') {
        return io::Error{io::ErrorKind::InvalidData};
      }
      _inn._read.consume(1);
    }

    const auto c = _TRY(_inn.peak());
    if (c == '}') {
      _ended = true;
      return Option<Item>{};
    }

    if (_count++ != 0) {
      if (c != ',') {
        return io::Error{io::ErrorKind::InvalidData};
      }
      _inn._read.consume(1);
    }
    return Option<Item>{Item{_inn}};
  }
};

auto to_writer(auto& writer, const auto& val) {
  auto ser = Serializer{writer};
  return Serialize::serialize(val, ser);
}

auto to_string(const auto& val) -> String {
  auto s = String{};
  auto& w = trait::as_mut<io::Write>(s);
  json::to_writer(w, val);
  return s;
}

}  // namespace sfc::serde::json
