#pragma once

#include "sfc/alloc.h"

namespace sfc::serde::json {

enum class Token {
  Comma,        // ','
  Colon,        // ':'
  Null,         // 'null'
  True,         // 'true'
  False,        // 'false'
  String,       // string ".*"
  Number,       // number [0-9*]
  ArrayBegin,   // '['
  ArrayEnd,     // ']'
  ObjectBegin,  // '{'
  ObjectEnd,    // '}'
  Other,        // other
};

enum class Error {
  Success,
  InvalidToken,         // invalid token
  UnexpectedEof,        // unexpected end of file
  ExpectedComma,        // expected ','
  ExpectedColon,        // expected ':'
  ExpectedArrayBegin,   // expected '['
  ExpectedArrayEnd,     // expected ']'
  ExpectedObjectBegin,  // expected '{'
  ExpectedObjectEnd,    // expected '}'
  InvalidNumber,        // invalid number format
  InvalidString,        // invalid string format (e.g. invalid escape sequence)
};
auto to_str(Error err) -> Str;

template <class T = Unit>
using Result = result::Result<T, Error>;

class Serializer;
class SerializeSeq;
class SerializeObj;

class Deserializer;

class Serializer {
  friend class SerializeSeq;
  friend class SerializeObj;
  String _buf;

  void write_tok(Token tok);

 public:
  auto as_str() const -> Str;
  auto into_string() && -> String;

  void serialize_null();
  void serialize_bool(bool val);
  void serialize_str(Str val);
  void serialize_num(trait::num_ auto val);

  auto serialize_seq() -> SerializeSeq;
  auto serialize_obj() -> SerializeObj;

  template <class T>
  void serialize_any(const T& val) {
    if constexpr (requires { val.serialize(*this); }) {
      return val.serialize(*this);
    } else if constexpr (trait::same_<T, bool>) {
      return this->serialize_bool(val);
    } else if constexpr (trait::num_<T>) {
      return this->serialize_num(val);
    } else if constexpr (requires { Str{val}; }) {
      return this->serialize_str(val);
    } else if constexpr (requires { Slice{val}; }) {
      return Slice{val}.serialize(*this);
    } else {
      static_assert(false, "Serialize::serialize: not serializable");
    }
  }
};

class SerializeSeq {
  Serializer& _ser;
  usize _count = 0;

  void serialize_imp();

 public:
  explicit SerializeSeq(Serializer& ser);
  ~SerializeSeq();
  SerializeSeq(const SerializeSeq&) = delete;
  void operator=(const SerializeSeq&) = delete;

  void serialize_element(const auto& item) {
    this->serialize_imp();
    _ser.serialize_any(item);
  }
};

class SerializeObj {
  Serializer& _ser;
  usize _count{0};

 public:
  explicit SerializeObj(Serializer& ser);
  ~SerializeObj();
  SerializeObj(const SerializeObj&) = delete;
  SerializeObj& operator=(const SerializeObj&) = delete;

  void serialize_key(Str key);

  void serialize_value(const auto& val) {
    _ser.serialize_any(val);
  }

  void serialize_entry(Str key, const auto& val) {
    this->serialize_key(key);
    this->serialize_value(val);
  }
};

class Deserializer {
  Str _buf;

  auto peek_tok() -> Result<Token>;
  auto next_tok() -> Result<Str>;
  auto read_tok(Token expected) -> Result<>;

 public:
  template <class T = Unit>
  using Result = json::Result<T>;

  static auto from_str(Str buf) -> Deserializer;

  auto deserialize_null() -> Result<>;
  auto deserialize_bool() -> Result<bool>;
  auto deserialize_num() -> Result<Str>;
  auto deserialize_str() -> Result<Str>;

  class DeserializeSeq;

  auto deserialize_seq() -> Result<DeserializeSeq>;

  class DeserializeObj;
  auto deserialize_obj() -> Result<DeserializeObj>;
  auto deserialize_dict() -> Result<DeserializeObj>;

  template <class T>
  auto deserialize_any() -> Result<T> {
    if constexpr (requires { T::deserialize(*this); }) {
      return T::deserialize(*this);
    } else if constexpr (trait::same_<T, bool>) {
      return this->deserialize_bool();
    } else if constexpr (trait::int_<T> || trait::flt_<T>) {
      const auto num_str = _TRY(this->deserialize_num());
      return num_str.parse<T>().ok_or(json::Error::InvalidNumber);
    } else if constexpr (trait::same_<T, String>) {
      return this->deserialize_str();
    } else {
      static_assert(false, "json::Deserializer::deserialize: not deserializable");
    }
  }
};

class Deserializer::DeserializeSeq {
  Deserializer& _des;
  usize _count{0};
  bool _finished{false};

 public:
  auto end() -> Result<>;

  template <class T>
  auto next_element() -> Result<Option<T>> {
    const auto has_next = _TRY(this->next_imp());
    if (!has_next) {
      return Option<T>{};
    }
    auto element = _TRY(_des.deserialize_any<T>());
    return {mem::move(element)};
  }

  template <class T>
  auto for_each(auto&& f) -> Result<> {
    while (true) {
      const auto has_next = _TRY(this->next_imp());
      if (!has_next) {
        break;
      }
      auto element = _TRY(_des.deserialize_any<T>());
      f(mem::move(element));
    }
    return Ok{};
  }

 private:
  friend class Deserializer;
  DeserializeSeq(Deserializer& inn);
  auto next_imp() -> Result<bool>;
};

class Deserializer::DeserializeObj {
  Deserializer& _des;
  usize _count{0};
  bool _finished{false};

 public:
  auto end() -> Result<>;

  template <class T>
  auto next_val() -> Result<T> {
    _TRY(_des.read_tok(Token::Colon));
    return _des.deserialize_any<T>();
  }

  template <class T>
  auto for_each(auto&& f) -> Result<> {
    auto key = String{};
    while (true) {
      key.clear();
      auto key_opt = _TRY(this->next_key());
      if (!key_opt) {
        break;
      }
      key.push_str(*key_opt);

      auto val = _TRY(this->next_val<T>());
      f(key.as_str(), mem::move(val));
    }
    _TRY(this->end());
    return Ok{};
  }

 private:
  friend class Deserializer;

  DeserializeObj(Deserializer& inn);
  auto next_imp() -> Result<bool>;
  auto next_key() -> Result<Option<Str>>;
};

void to_writer(auto& writer, const auto& val) {
  auto ser = Serializer{writer};
  ser.serialize_any(val);
}

auto to_string(const auto& val) -> String {
  auto ser = Serializer{};
  ser.serialize_any(val);
  return mem::move(ser).into_string();
}

}  // namespace sfc::serde::json
