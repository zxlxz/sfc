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

 public:
  explicit Serializer(String buf);
  ~Serializer();
  Serializer(Serializer&&) noexcept;
  Serializer& operator=(Serializer&&) noexcept;

  static auto new_() -> Serializer;

 public:
  auto as_str() const -> Str;
  auto into_string() && -> String;

 public:
  void write_tok(Token tok);

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

 public:
  void serialize_key(Str key);

  void serialize_value(const auto& val) {
    _ser.serialize_any(val);
  }

  void serialize_entry(const auto& key, const auto& val) {
    this->serialize_key(str::AsStr::as_str(key));
    this->serialize_value(val);
  }
};

class DeserializeSeq;
class DeserializeDict;

class Deserializer {
  Str _buf;

  friend class DeserializeSeq;
  friend class DeserializeDict;

  auto peek_tok() -> Result<Token>;
  auto next_tok() -> Result<Str>;
  auto read_tok(Token expected) -> Result<>;

 public:
  using Error = json::Error;

  static auto from_str(Str buf) -> Deserializer;

  auto deserialize_null() -> Result<>;
  auto deserialize_bool() -> Result<bool>;
  auto deserialize_num() -> Result<Str>;
  auto deserialize_str() -> Result<Str>;

  auto as_seq() -> DeserializeSeq;
  auto as_dict() -> DeserializeDict;

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

class DeserializeSeq {
  Deserializer& _des;
  usize _count{0};
  bool _finished{false};

 private:
  friend class Deserializer;
  DeserializeSeq(Deserializer& des) : _des(des) {}

  auto next_entry() -> Result<>;

 public:
  template <class T>
  auto for_each(auto&& f) && -> Result<> {
    while (!_finished) {
      _TRY(this->next_entry());
      if (!_finished) {
        auto val = _TRY(_des.deserialize_any<T>());
        f(mem::move(val));
      }
    }
    return Ok{};
  }
};

class DeserializeDict {
  Deserializer& _des;
  usize _count{0};
  bool _finished{false};

 private:
  friend class Deserializer;
  DeserializeDict(Deserializer& des) : _des(des) {}

  auto next_key() -> Result<Str>;

 public:
  template <class T>
  auto for_each(auto&& f) && -> Result<> {
    while (!_finished) {
      const auto key = _TRY(this->next_key());
      if (!_finished) {
        auto val = _TRY(_des.deserialize_any<T>());
        f(key, mem::move(val));
      }
    }
    return Ok{};
  }
};

void to_writer(auto& writer, const auto& val) {
  auto ser = Serializer{writer};
  ser.serialize_any(val);
}

auto to_string(const auto& val) -> String {
  auto ser = Serializer::new_();
  ser.serialize_any(val);
  return mem::move(ser).into_string();
}

}  // namespace sfc::serde::json
