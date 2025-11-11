#pragma once

#include "sfc/serde/base64.h"

namespace sfc::serde::json {

template <class W>
class Serializer {
  W& _write;

 public:
  explicit Serializer(W& write) noexcept : _write{write} {}
  ~Serializer() noexcept = default;
  Serializer(const Serializer&) noexcept = delete;

 public:
  template <class T>
  auto serialize(const T& val) {
    if constexpr (requires { val.serialize(*this); }) {
      return val.serialize(*this);
    } else if constexpr (trait::same_<T, bool>) {
      return this->serialize_bool(val);
    } else if constexpr (trait::same_<T, char>) {
      return this->serialize_char(val);
    } else if constexpr (trait::int_<T>) {
      return this->serialize_int(val);
    } else if constexpr (trait::flt_<T>) {
      return this->serialize_flt(val);
    } else if constexpr (requires { Str{val}; }) {
      return this->serialize_str(val);
    } else if constexpr (requires { Slice{val}; }) {
      return Slice{val}.serialize(*this);
    } else {
      static_assert(false, "Serializer::serialize: not serializable");
    }
  }

  void serialize_null() {
    const auto s = "null";
    _write.write_str(s);
  }

  auto serialize_bool(const bool& val) {
    const auto s = val ? Str{"true"} : Str{"false"};
    return _write.write_str(s);
  }

  auto serialize_char(const char& val) {
    const char s[] = {'"', val, '"'};
    return _write.write_str({s, 3});
  }

  auto serialize_int(const trait::int_ auto& val) {
    char buf[32] = {};
    const auto s = fmt::Debug::fmt_int(buf, val);
    return _write.write_str(s);
  }

  auto serialize_flt(const trait::flt_ auto& val) {
    char buf[32] = {};
    const auto s = fmt::Debug::fmt_flt(buf, val);
    return _write.write_str(s);
  }

  auto serialize_str(Str val) {
    _write.write_str("\"");
    _write.write_str(val);
    return _write.write_str("\"");
  }

  auto serialize_bin(Slice<const u8> val) {
    const auto s = base64::encode(val);
    return this->serialize_str(s);
  }

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
  explicit SerSeq(Serializer& inn) noexcept : _inn{inn} {
    _inn._write.write_str("[");
  }

  ~SerSeq() noexcept {
    _inn._write.write_str("]");
  }

  SerSeq(const SerSeq&) noexcept = delete;

  void serialize_element(const auto& item) {
    if (_cnt++ != 0) {
      _inn._write.write_str(",");
    }
    _inn.serialize(item);
  }
};

template <class W>
class Serializer<W>::SerMap {
  Serializer& _inn;
  u32 _cnt = 0;

 public:
  explicit SerMap(Serializer& inn) noexcept : _inn{inn} {
    _inn._write.write_str("{");
  }

  ~SerMap() noexcept {
    _inn._write.write_str("}");
  }

  SerMap(const SerMap&) noexcept = delete;

  void serialize_entry(Str key, const auto& val) {
    if (_cnt++ != 0) {
      _inn._write.write_str(",");
    }
    _inn._write.write_str("\"");
    _inn._write.write_str(key);
    _inn._write.write_str("\":");
    return _inn.serialize(val);
  }
};

template <class R>
class Deserializer {
  static_assert(!trait::same_<R, const R>);
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
    return Str::from_utf8(b[{0, n}]);
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
    return Str::from_utf8(buf[{0, n}]);
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

void to_writer(auto& writer, const auto& val) {
  auto ser = Serializer{writer};
  Serialize::serialize(val, ser);
}

auto to_string(const auto& val) -> String {
  auto buf = String{};
  auto ser = Serializer{buf};
  Serialize::serialize(val, ser);
  return buf;
}

}  // namespace sfc::serde::json
