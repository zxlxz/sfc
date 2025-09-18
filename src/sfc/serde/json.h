#pragma once

#include "sfc/serde/mod.h"

namespace sfc::serde::json {

template <class Ser>
class SerializeSeq {
  Ser& _ser;
  u32 _cnt = 0;

 public:
  explicit SerializeSeq(Ser& ser) noexcept : _ser{ser} {
    _ser._write.write_char('[');
  }

  ~SerializeSeq() noexcept {
    _ser._write.write_char(']');
  }

  SerializeSeq(const SerializeSeq&) noexcept = delete;

  void serialize_element(const auto& item) {
    if (_cnt++ != 0) {
      _ser._write.write_char(',');
    }
    Serialize::serialize(item, _ser);
  }
};

template <class Ser>
class SerializeMap {
  Ser& _ser;
  u32 _cnt = 0;

 public:
  explicit SerializeMap(Ser& ser) noexcept : _ser{ser} {
    _ser._write.write_char('{');
  }

  ~SerializeMap() noexcept {
    _ser._write.write_char('}');
  }

  SerializeMap(const SerializeMap&) noexcept = delete;

  void serialize_field(str::Str key, const auto& value) {
    if (_cnt++ != 0) {
      _ser._write.write_char(',');
    }
    _ser._write.write_char('"');
    _ser._write.write_str(key);
    _ser._write.write_char('"');
    _ser._write.write_char(':');
    Serialize::serialize(value, _ser);
  }
};

template <class W>
class Serializer {
  friend class SerializeSeq<Serializer>;
  friend class SerializeMap<Serializer>;

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

  auto serialize_seq() -> SerializeSeq<Serializer> {
    return SerializeSeq{*this};
  }

  auto serialize_map() -> SerializeMap<Serializer> {
    return SerializeMap{*this};
  }
};

template <class R>
class Deserializer {
  io::BufReader<R> _read;

 public:
  explicit Deserializer(R& read) noexcept : _read{read} {}
  ~Deserializer() noexcept = default;
  Deserializer(const Deserializer&) noexcept = delete;

 public:
  auto deserialize_null() -> io::Result<Str> {
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

  template <class T = int>
  auto deserialize_int() -> io::Result<T> {
    _TRY(this->peak());
    const auto s = this->pop_num();
    return s.template parse<T>();
  }

  template <class T = float>
  auto deserialize_flt() {
    _TRY(this->peak());
    const auto s = this->pop_num();
    return s.template parse<T>();
  }

  auto deserialize_str() -> io::Result<String> {
    const auto c = _TRY(this->peak());
    if (c != '"') {
      return io::Error{io::ErrorKind::InvalidData};
    }
    return this->pop_str();
  }

 private:
  auto peak() -> io::Result<char> {
    static const auto is_blank = +[](char c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); };
    _TRY(_read.skip(is_blank));
    const auto b = _TRY(_read.fill_buf());
    return b.is_empty() ? '\0' : char(b[0]);
  }

  auto pop_match(Str s) -> io::Result<Str> {
    const auto b = _TRY(_read.peak(s.len()));
    if (s != b) {
      return io::Error{io::ErrorKind::InvalidData};
    }
    return s;
  }

  auto pop_num() -> io::Result<Str> {
    static const auto is_digits = +[](char c) { return c == ' ' || ('\x09' <= c && c <= '\x0d'); };

    const auto b = _TRY(_read.peak(64));
    const auto n = b.find_if(is_digits).unwrap_or(b.len());
    if (n == 0) {
      return io::Error{io::ErrorKind::InvalidData};
    }
    _read.consume(n);
    return b.slice(0, n);
  }

  auto pop_str() -> io::Result<String> {
    _read.consume(1);  // consume the leading '"'
    auto res = String{};

    const auto cnt = _TRY(_read.read_until('"', res));
    if (cnt == 0 || res[cnt - 1] != '"') {
      return io::Error{io::ErrorKind::InvalidData};
    }
    res.pop();
    return res;
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
