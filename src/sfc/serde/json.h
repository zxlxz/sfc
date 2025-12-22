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
    _buf.push_str("null");
  }

  void serialize_bool(bool val) noexcept {
    _buf.push_str(val ? Str{"true"} : Str{"false"});
  }

  void serialize_char(char val) noexcept {
    const char s[] = {'"', val, '"'};
    _buf.push_str({s, 3});
  }

  void serialize_int(trait::int_ auto val) noexcept {
    char buf[32];
    const auto s = num::to_str(buf, val);
    _buf.push_str(s);
  }

  void serialize_flt(trait::flt_ auto val) noexcept {
    char buf[32];
    const auto s = num::to_str(buf, val);
    _buf.push_str(s);
  }

  void serialize_str(Str val) noexcept {
    _buf.push('"');
    _buf.push_str(val);
    _buf.push('"');
  }

  void serialize_bin(Slice<const u8> val) noexcept {
    const auto s = base64::encode(val);
    _buf.push('"');
    _buf.push_str(s.as_str());
    _buf.push('"');
  }

 public:
  struct SerArray {
    Serializer& _ser;
    u32 _cnt = 0;

   public:
    SerArray(Serializer& ser) : _ser{ser} {
      _ser._buf.push('[');
    }

    ~SerArray() {
      _ser._buf.push(']');
    }

    SerArray(const SerArray&) = delete;
    SerArray& operator=(const SerArray&) = delete;

    void serialize_element(const auto& item) noexcept {
      if (_cnt++ != 0) {
        _ser._buf.push(',');
      }
      Serialize::serialize(item, _ser);
    }
  };

  struct SerObject {
    Serializer& _ser;
    u32 _cnt = 0;

   public:
    SerObject(Serializer& ser) : _ser{ser} {
      _ser._buf.push('{');
    }

    ~SerObject() {
      _ser._buf.push('}');
    }

    SerObject(const SerObject&) = delete;
    SerObject& operator=(const SerObject&) = delete;

    void serialize_entry(Str key, const auto& val) noexcept {
      if (_cnt++ != 0) {
        _ser._buf.push(',');
      }
      _ser._buf.push('"');
      _ser._buf.push_str(key);
      _ser._buf.push_str("\":");
      Serialize::serialize(val, _ser);
    }
  };

  auto serialize_seq() noexcept -> SerArray {
    return SerArray{*this};
  }

  auto serialize_obj() noexcept -> SerObject {
    return SerObject{*this};
  }

  auto serialize_map() noexcept -> SerObject {
    return SerObject{*this};
  }
};

struct Deserializer {
  using Error = json::Error;
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

 public:
  struct DesArray {
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

  struct DesObject {
    using Error = json::Error;
    Deserializer& _inn;
    usize _idx = 0;

   public:
    auto has_next() noexcept -> bool {
      const auto next_tok = _inn.next_token();
      return next_tok != Token::ObjectEnd;
    }

    template <class K>
    auto next_key() noexcept -> Result<K> {
      static_assert(trait::same_<K, Str>, "DesObject::next_key: key type must be Str");

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

  auto deserialize_seq(auto&& visit) -> Result<> {
    if (this->extract_tok(Token::ArrayBegin).is_err()) {
      return Error::ExpectedArrayBegin;
    }
    auto imp = DesArray{*this};
    if (auto x = visit(imp); x.is_err()) {
      return ~x;
    }
    return this->extract_tok(Token::ArrayEnd);
  }

  auto deserialize_obj(auto&& visit) -> Result<> {
    if (this->extract_tok(Token::ObjectBegin).is_err()) {
      return Error::ExpectedObjectBegin;
    }
    auto imp = DesObject{*this};
    if (auto x = visit(imp); x.is_err()) {
      return ~x;
    }
    return this->extract_tok(Token::ObjectEnd);
  }

  auto deserialize_map(auto&& visit) -> Result<> {
    if (this->extract_tok(Token::ObjectBegin).is_err()) {
      return Error::ExpectedObjectBegin;
    }
    auto imp = DesObject{*this};
    if (auto x = visit(imp); x.is_err()) {
      return ~x;
    }
    return this->extract_tok(Token::ObjectEnd);
  }

 private:
  void consume(usize n) noexcept;
  auto next_token() noexcept -> Token;

  auto extract_tok(Token tok) noexcept -> Result<>;
  auto extract_num() noexcept -> Result<Str>;
  auto extract_str() noexcept -> Result<Str>;
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
