#pragma once

#include "sfc/alloc/string.h"

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

enum class Error {
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

  void serialize_int(num::int_ auto val) noexcept {
    char buf[32];
    const auto s = fmt::Debug::format_int(buf, val);
    _buf.push_str(s);
  }

  void serialize_flt(num::float_ auto val) noexcept {
    char buf[32];
    const auto s = fmt::Debug::format_flt(buf, val, 6);
    _buf.push_str(s);
  }

  void serialize_str(Str val) noexcept {
    _buf.push('"');
    _buf.push_str(val);
    _buf.push('"');
  }

  template <class T>
  void serialize_any(const T& val) {
    if constexpr (requires { val.serialize(*this); }) {
      return val.serialize(*this);
    } else if constexpr (same_<T, bool>) {
      return this->serialize_bool(val);
    } else if constexpr (same_<T, char>) {
      return this->serialize_char(val);
    } else if constexpr (num::int_<T>) {
      return this->serialize_int(val);
    } else if constexpr (num::float_<T>) {
      return this->serialize_flt(val);
    } else if constexpr (requires { Str{val}; }) {
      return this->serialize_str(val);
    } else if constexpr (requires { Slice{val}; }) {
      return Slice{val}.serialize(*this);
    } else {
      static_assert(false, "Serialize::serialize: not serializable");
    }
  }

 public:
  struct SerArray {
    Serializer& _ser;
    u32 _cnt = 0;

   public:
    explicit SerArray(Serializer& ser) : _ser{ser} {
      _ser._buf.push('[');
    }

    ~SerArray() {
      _ser._buf.push(']');
    }

    SerArray(const SerArray&) = delete;
    void operator=(const SerArray&) = delete;

    void serialize_element(const auto& item) noexcept {
      if (_cnt++ != 0) {
        _ser._buf.push(',');
      }
      _ser.serialize_any(item);
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
      _ser._buf.push('"');
      _ser._buf.push(':');
      _ser.serialize_any(val);
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

  auto deserialize_char() noexcept -> Result<char> {
    const auto s = _TRY(this->extract_str());
    if (s.len() != 1) {
      return Error::InvalidString;
    }
    return s[0];
  }

  template <num::int_ T>
  auto deserialize_int() noexcept -> Result<T> {
    const auto num_str = _TRY(this->extract_num());
    const auto num_val = num_str.template parse<T>();
    if (!num_val) {
      return Error::InvalidNumber;
    }
    return *num_val;
  }

  template <num::float_ T>
  auto deserialize_flt() -> Result<T> {
    const auto num_str = _TRY(this->extract_num());
    const auto num_val = num_str.template parse<T>();
    if (!num_val) {
      return Error::InvalidNumber;
    }
    return *num_val;
  }

  auto deserialize_str() noexcept -> Result<Str> {
    return this->extract_str();
  }

  template <class T>
  auto deserialize_any() noexcept -> Result<T> {
    if constexpr (requires { T::deserialize(*this); }) {
      return T::deserialize(*this);
    } else if constexpr (same_<T, bool>) {
      return this->deserialize_bool();
    } else if constexpr (same_<T, char>) {
      return this->deserialize_char();
    } else if constexpr (num::int_<T>) {
      return this->template deserialize_int<T>();
    } else if constexpr (num::float_<T>) {
      return this->template deserialize_flt<T>();
    } else if constexpr (requires { T{Str{}}; }) {
      return this->deserialize_str().and_then([](Str s) { return T{static_cast<Str&&>(s)}; });
    } else {
      static_assert(false, "Deserialize::deserialize: not deserializable");
    }
  }

 public:
  struct DesArray {
    using Error = json::Error;
    Deserializer& _inn;
    usize _idx = 0;

   public:
    auto next() noexcept -> bool {
      const auto next_tok = _inn.next_token();
      return next_tok != Token::ArrayEnd;
    }

    template <class T>
    auto next_element() noexcept -> Result<T> {
      if (_idx != 0 && _inn.extract_tok(Token::Comma).is_err()) {
        return Error::ExpectedComma;
      }
      _idx += 1;
      return _inn.deserialize_any<T>();
    }
  };

  struct DesObject {
    using Error = json::Error;
    Deserializer& _inn;
    usize _idx = 0;

   public:
    auto next() noexcept -> bool {
      const auto next_tok = _inn.next_token();
      return next_tok != Token::ObjectEnd;
    }

    template <class K>
    auto next_key() noexcept -> Result<K> {
      static_assert(same_<K, Str>, "DesObject::next_key: key type must be Str");

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
      return _inn.deserialize_any<T>();
    }
  };

  auto deserialize_seq(auto&& visit) -> Result<> {
    if (this->extract_tok(Token::ArrayBegin).is_err()) {
      return Error::ExpectedArrayBegin;
    }
    auto imp = DesArray{*this};
    _TRY(visit(imp));
    return this->extract_tok(Token::ArrayEnd);
  }

  auto deserialize_obj(auto&& visit) -> Result<> {
    if (this->extract_tok(Token::ObjectBegin).is_err()) {
      return Error::ExpectedObjectBegin;
    }
    auto imp = DesObject{*this};
    _TRY(visit(imp));
    return this->extract_tok(Token::ObjectEnd);
  }

  auto deserialize_map(auto&& visit) -> Result<> {
    if (this->extract_tok(Token::ObjectBegin).is_err()) {
      return Error::ExpectedObjectBegin;
    }
    auto imp = DesObject{*this};
    _TRY(visit(imp));
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
  ser.serialize_any(val);
}

auto to_string(const auto& val) -> String {
  auto buf = String{};
  auto ser = Serializer{buf};
  ser.serialize_any(val);
  return buf;
}

}  // namespace sfc::serde::json
