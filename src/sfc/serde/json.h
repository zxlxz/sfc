#pragma once

#include "sfc/serde/base64.h"

namespace sfc::serde::json {

enum class Error : i8 {
  Success,
  IoError,                // I/O error
  EofWhileParsing,        // EOF while parsing keyword
  ExpectedComma,          // expected ','
  ExpectedDoubleQuote,    // expected '"'
  ExpectedColon,          // expected ':'
  ExpectedBeginOfList,    // expected '['
  ExpectedEndOfList,      // expected ']'
  ExpectedBeginOfObject,  // expected '{'
  ExpectedEndOfObject,    // expected '}'
  InvalidKeyword,         // invalid keyword (true, false, null)
  InvalidNumber,          // invalid number format
  InvalidString,          // invalid string format (e.g. invalid escape sequence)
};

template <class T = void>
using Result = result::Result<T, Error>;

template <class W>
class Serializer {
  template <class T>
  using Result = result::Result<T, Error>;

  W& _write;

 public:
  explicit Serializer(W& write) noexcept : _write{write} {}
  ~Serializer() noexcept = default;
  Serializer(const Serializer&) noexcept = delete;

 public:
  void serialize_null() {
    const auto s = "null";
    _write.write_str(s);
  }

  void serialize_bool(bool val) {
    const auto s = val ? Str{"true"} : Str{"false"};
    return this->write_str(s);
  }

  void serialize_char(char val) {
    const char s[] = {'"', val, '"'};
    _write.write_str({s, 3});
  }

  void serialize_int(trait::int_ auto val) {
    char buf[32] = {};
    const auto s = fmt::Debug::to_str(buf, val);
    this->write_str(s);
  }

  void serialize_flt(trait::flt_ auto val) {
    char buf[32] = {};
    const auto s = fmt::Debug::to_str(buf, val);
    this->write_str(s);
  }

  void serialize_str(Str val) {
    this->write_str("\"");
    this->write_str(val);
    this->write_str("\"");
  }

  void serialize_bin(Slice<const u8> val) {
    const auto s = base64::encode(val);
    this->serialize_str(s);
  }

  struct SerSeq;
  auto serialize_seq() -> SerSeq {
    this->write_str("[");
    return SerSeq{*this};
  }

  struct SerMap;
  auto serialize_map() -> SerMap {
    return SerMap{this};
  }

 private:
  void write_str(Str s) {
    _write.write_str(s).unwrap();
  }
};

template <class W>
class Serializer<W>::SerSeq {
  Serializer* _ser;
  u32 _cnt = 0;

 public:
  SerSeq(Serializer& ser) noexcept : _ser{&ser} {}
  ~SerSeq() noexcept = default;

  SerSeq(const SerSeq&) = delete;
  SerSeq& operator=(const SerSeq&) = delete;

  void serialize_element(const auto& item) {
    if (_ser == nullptr) {
      return;
    }
    if (_cnt++ != 0) {
      _ser->write_str(",");
    }
    Serialize::serialize(item, *_ser);
  }

  void end() {
    if (_ser == nullptr) {
      return;
    }
    _ser->write_str("]");
    _ser = nullptr;
  }
};

template <class W>
class Serializer<W>::SerMap {
  Serializer* _inn = nullptr;
  u32 _cnt = 0;

 public:
  void serialize_entry(Str key, const auto& val) {
    if (_inn == nullptr) {
      return;
    }
    if (_cnt++ != 0) {
      _inn->write_str(",");
    }
    _inn->write_str("\"");
    _inn->write_str(key);
    _inn->write_str("\":");
    Serialize::serialize(val, *_inn);
  }
};

template <class R>
class Deserializer {
  io::BufReader<R> _read;

 public:
  explicit Deserializer(R&& read) noexcept : _read{static_cast<R&&>(read)} {}
  ~Deserializer() noexcept = default;
  Deserializer(const Deserializer&) noexcept = delete;

 public:
  auto deserialize_null() -> Result<> {
    const auto c = this->peak();
    if (c == 'n') {
      return this->extract_keyword("null");
    }
    return c ? Error::InvalidKeyword : Error::EofWhileParsing;
  }

  auto deserialize_bool() -> Result<bool> {
    const auto c = this->peak();
    if (c == 't') {
      return this->extract_keyword("true") & Result<bool>{true};
    } else if (c == 'f') {
      return this->extract_keyword("false") & Result<bool>{false};
    }
    return c ? Error::InvalidKeyword : Error::EofWhileParsing;
  }

  template <trait::int_ T>
  auto deserialize_int() -> Result<T> {
    const auto c = this->peak();
    if (c == '+' || c == '-' || ('0' <= c && c <= '9')) {
      return this->extract_num<T>();
    }
    return c ? Error::InvalidNumber : Error::EofWhileParsing;
  }

  template <trait::flt_ T>
  auto deserialize_flt() -> Result<T> {
    const auto c = this->peak();
    if (c == '+' || c == '-' || ('0' <= c && c <= '9')) {
      return this->extract_num<T>();
    }
    return c ? Error::InvalidNumber : Error::EofWhileParsing;
  }

  auto deserialize_string() -> Result<String> {
    const auto c = this->peak();
    if (c == '"') {
      return this->extract_string();
    }
    return c ? Error::ExpectedDoubleQuote : Error::EofWhileParsing;
  }

  class DesSeq;
  auto deserialize_seq() -> Result<DesSeq> {
    const auto c = this->peak();
    if (c == '[') {
      return DesSeq{*this};
    }
    return c ? Error::ExpectedBeginOfList : Error::EofWhileParsing;
  }

  class DesMap;
  auto deserialize_map() -> Result<DesMap> {
    const auto c = this->peak();
    if (c == '{') {
      return DesMap{*this};
    }
    return c ? Error::ExpectedBeginOfObject : Error::EofWhileParsing;
  }

 private:
  void skip_blanks() {
    const auto not_blank = +[](char c) { return !(c == ' ' || ('\x09' <= c && c <= '\x0d')); };
    _read.skip_until(not_blank);
  }

  auto peak() -> char {
    this->skip_blanks();
    if (auto bytes = _read.peak(1)) {
      return bytes[0];
    }
    return 0;
  }

  auto extract_keyword(Str s) -> Result<> {
    const auto bytes = _read.peak(s.len());
    if (bytes == s.as_bytes()) {
      _read.consume(s.len());
      return {};
    }
    return bytes.is_empty() ? Error::EofWhileParsing : Error::InvalidKeyword;
  }

  template <class T>
  auto extract_num() -> Result<T> {
    static const auto not_digits = +[](char c) { return !('0' <= c && c <= '9' || c == '.' || c == '+' || c == '-'); };

    const auto bytes = _read.peak(64U);
    const auto position = bytes.iter().position(not_digits).unwrap_or(bytes.len());
    const auto digits = Str::from_utf8(bytes[{0, position}]);
    const auto number = digits.template parse<T>();
    if (!number) {
      return Error::InvalidNumber;
    }
    _read.consume(digits.len());
    return *number;
  }

  auto extract_string() -> Result<String> {
    _read.consume(1);

    auto res = String{};
    if (auto x = _read.read_until('"', res.as_mut_vec()); x.is_err()) {
      return Error::EofWhileParsing;
    }
    res.pop();
    return res;
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

    template <class T>
    auto extract_item() -> io::Result<T> {
      return _inn.template deserialize<T>();
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

    auto extract_key() -> io::Result<String> {
      auto s = _TRY(_inn.deserialize_string());
      return static_cast<String&&>(s);
    }

    template <class T>
    auto extract_val() -> io::Result<T> {
      const auto c = _TRY(_inn.peak());
      if (c != ':') {
        return io::Error{io::ErrorKind::InvalidData};
      }
      _inn._read.consume(1);
      return _inn.template deserialize<T>();
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

template <class R>
Deserializer(R&) -> Deserializer<R&>;

template <usize N>
Deserializer(const char (&)[N]) -> Deserializer<Str>;

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
