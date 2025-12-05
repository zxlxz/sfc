#pragma once

#include "sfc/serde/base64.h"

namespace sfc::serde::json {

enum class Token {
  Eof,
  Comma,        // ','
  Colon,        // ':'
  DoubleQuote,  // '"'
  ArrayBegin,   // '['
  ArrayEnd,     // ']'
  ObjectBegin,  // '{'
  ObjectEnd,    // '}'
  Null,         // 'null'
  True,         // 'true'
  False,        // 'false'
  Other,        // number, etc.
};

enum class Error : i8 {
  Success,
  EofWhileParsing,      // EOF while parsing keyword
  ExpectedComma,        // expected ','
  ExpectedDoubleQuote,  // expected '"'
  ExpectedColon,        // expected ':'
  ExpectedArrayBegin,   // expected '['
  ExpectedArrayEnd,     // expected ']'
  ExpectedObjectBegin,  // expected '{'
  ExpectedObjectEnd,    // expected '}'
  InvalidKeyword,       // invalid keyword (true, false, null)
  InvalidNumber,        // invalid number format
  InvalidString,        // invalid string format (e.g. invalid escape sequence)
};

template <class T = void>
using Result = result::Result<T, Error>;

struct Serializer {
  String& _buf;

 public:
  void serialize_null() noexcept {
    this->write_str("null");
  }

  void serialize_bool(bool val) noexcept {
    this->write_tok(val ? Token::True : Token::False);
  }

  void serialize_char(char val) noexcept {
    const char s[] = {'"', val, '"'};
    this->write_str({s, 3});
  }

  void serialize_int(trait::int_ auto val) noexcept {
    char buf[32];
    const auto s = num::to_str(buf, val);
    this->write_num(s);
  }

  void serialize_flt(trait::flt_ auto val) noexcept {
    char buf[32];
    const auto s = num::to_str(buf, val);
    this->write_num(s);
  }

  void serialize_str(Str val) noexcept {
    this->write_str(val);
  }

  void serialize_bin(Slice<const u8> val) noexcept {
    const auto s = base64::encode(val);
    this->serialize_str(s);
  }

  class SerArray;
  auto serialize_seq() noexcept -> SerArray;

  class SerObject;
  auto serialize_map() noexcept -> SerObject;

 private:
  void write_tok(Token tok) noexcept;
  void write_str(Str s) noexcept;
  void write_key(Str s) noexcept;
  void write_num(Str s) noexcept;
};

class Serializer::SerArray {
  Serializer& _inn;
  u32 _cnt = 0;

 public:
  explicit SerArray(Serializer& inn) noexcept : _inn{inn} {
    _inn.write_tok(Token::ArrayBegin);
  }

  ~SerArray() noexcept {
    _inn.write_tok(Token::ArrayEnd);
  }

  SerArray(const SerArray&) = delete;

  void serialize_element(const auto& item) noexcept {
    if (_cnt++ != 0) {
      _inn.write_tok(Token::Comma);
    }
    Serialize::serialize(item, _inn);
  }
};

class Serializer::SerObject {
  Serializer& _inn;
  u32 _cnt = 0;

 public:
  SerObject(Serializer& inn) noexcept : _inn{inn} {
    _inn.write_tok(Token::ObjectBegin);
  }

  ~SerObject() noexcept {
    _inn.write_tok(Token::ObjectEnd);
  }

  SerObject(const SerObject&) = delete;

  void serialize_entry(Str key, const auto& val) noexcept {
    if (_cnt++ != 0) {
      _inn.write_tok(Token::Comma);
    }
    _inn.write_key(key);
    Serialize::serialize(val, _inn);
  }
};

struct Deserializer {
  Str _buf;

 public:
  auto deserialize_null() noexcept -> Result<> {
    return this->extract_tok(Token::Null);
  }

  auto deserialize_bool() noexcept -> Result<bool> {
    const auto next_tok = this->next_token();
    switch (next_tok) {
      case Token::True:  this->consume(4); return true;
      case Token::False: this->consume(5); return false;
      case Token::Eof:   return Error::EofWhileParsing;
      default:           return Error::InvalidKeyword;
    }
  }

  template <trait::int_ T>
  auto deserialize_int() noexcept -> Result<T> {
    const auto num_str = this->extract_num();
    if (num_str.is_err()) {
      return ~num_str;
    }
    const auto num_val = num_str->template parse<T>();
    if (!num_val) {
      return Error::InvalidNumber;
    }
    return *num_val;
  }

  template <trait::flt_ T>
  auto deserialize_flt() -> Result<T> {
    const auto num_str = this->extract_num();
    if (num_str.is_err()) {
      return ~num_str;
    }
    const auto num_val = num_str->template parse<T>();
    if (!num_val) {
      return Error::InvalidNumber;
    }
    return *num_val;
  }

  auto deserialize_str() noexcept -> Result<Str> {
    return this->extract_str();
  }

  struct DesArray;
  auto deserialize_seq(auto&& f) -> Result<>;

  struct DesObject;
  auto deserialize_map(auto&& f) -> Result<>;

 private:
  void consume(usize n) noexcept;
  auto next_token() noexcept -> Token;

  auto extract_tok(Token tok) noexcept -> Result<>;
  auto extract_num() noexcept -> Result<Str>;
  auto extract_str() noexcept -> Result<Str>;
};

struct Deserializer::DesArray {
  using Error = json::Error;
  Deserializer& _inn;
  usize _idx = 0;

 public:
  auto has_next() noexcept -> bool {
    const auto next_tok = _inn.next_token();
    return next_tok != Token::ArrayEnd;
  }

  template <class T>
  auto next_element() noexcept -> Result<T> {
    if (_idx != 0 && _inn.extract_tok(Token::Comma).is_err()) {
      return Error::ExpectedComma;
    }
    _idx += 1;
    return Deserialize::deserialize<T>(_inn);
  }
};

auto Deserializer::deserialize_seq(auto&& visit) -> Result<> {
  if (this->extract_tok(Token::ArrayBegin).is_err()) {
    return Error::ExpectedArrayBegin;
  }
  auto imp = DesArray{*this};
  if (auto x = visit(imp); x.is_err()) {
    return ~x;
  }
  return this->extract_tok(Token::ArrayEnd);
}

struct Deserializer::DesObject {
  using Error = json::Error;
  Deserializer& _inn;
  usize _idx = 0;

 public:
  auto has_next() noexcept -> bool {
    const auto next_tok = _inn.next_token();
    return next_tok != Token::ObjectEnd;
  }

  auto next_key() noexcept -> Result<Str> {
    if (_idx != 0 && _inn.extract_tok(Token::Comma).is_err()) {
      return Error::ExpectedComma;
    }
    _idx += 1;
    return _inn.extract_str();
  }

  template <class T>
  auto next_value() noexcept -> Result<T> {
    if (_inn.extract_tok(Token::Colon).is_err()) {
      return Error::ExpectedColon;
    }
    return Deserialize::deserialize<T>(_inn);
  }
};

auto Deserializer::deserialize_map(auto&& visit) -> Result<> {
  if (this->extract_tok(Token::ObjectBegin).is_err()) {
    return Error::ExpectedObjectBegin;
  }
  auto imp = DesObject{*this};
  if (auto x = visit(imp); x.is_err()) {
    return ~x;
  }
  return this->extract_tok(Token::ObjectEnd);
}

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
