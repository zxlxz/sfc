#pragma once

#include "sfc/serde/mod.h"
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
  void serialize_int(auto val);
  void serialize_flt(auto val);
  void serialize_str(Str val);
  void serialize_num(trait::num_ auto val);

  auto serialize_seq() -> SerializeSeq;
  auto serialize_obj() -> SerializeObj;
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
    Serialize{item}.serialize(_ser);
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
    Serialize{val}.serialize(_ser);
  }

  void serialize_entry(const auto& key, const auto& val) {
    this->serialize_key(str::AsStr::as_str(key));
    this->serialize_value(val);
  }
};

class DeserializeSeq;
class DeserializeMap;
class DeserializeObj;

class Deserializer {
  Str _buf;

  friend class DeserializeSeq;
  friend class DeserializeMap;
  friend class DeserializeObj;

  auto peek_tok() -> Result<Token>;
  auto next_tok() -> Result<Str>;
  auto read_tok(Token expected) -> Result<>;

 public:
  using Error = json::Error;

  static auto from_str(Str buf) -> Deserializer;

  auto deserialize_null() -> Result<>;
  auto deserialize_bool() -> Result<bool>;

  template<class T>
  auto deserialize_int() -> Result<T>;

  template<class T>
  auto deserialize_flt() -> Result<T>;
  auto deserialize_str() -> Result<Str>;

  auto as_seq() -> DeserializeSeq;
  auto as_map() -> DeserializeMap;
  auto as_obj() -> DeserializeObj;
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
        auto val = _TRY(Deserialize<T>::deserialize(_des));
        f(mem::move(val));
      }
    }
    return Ok{};
  }
};

class DeserializeMap {
  Deserializer& _des;
  usize _count{0};
  bool _finished{false};

 private:
  friend class Deserializer;
  DeserializeMap(Deserializer& des) : _des(des) {}

  auto next_key() -> Result<Str>;

 public:
  template <class T>
  auto for_each(auto&& f) && -> Result<> {
    while (!_finished) {
      const auto key = _TRY(this->next_key());
      if (!_finished) {
        auto val = _TRY(Deserialize<T>::deserialize(_des));
        f(key, mem::move(val));
      }
    }
    return Ok{};
  }
};

class DeserializeObj {
  Deserializer& _des;
  usize _count{0};
  bool _finished{false};

 private:
  friend class Deserializer;
  DeserializeObj(Deserializer& des) : _des(des) {}

  auto next_key() -> Result<Str>;

 public:
  auto for_each(auto&& f) && -> Result<> {
    while (!_finished) {
      const auto key = _TRY(this->next_key());
      if (!_finished) {
        f(key, _des);
      }
    }
    return Ok{};
  }
};

void to_writer(auto& writer, const auto& val) {
  auto ser = Serializer{writer};
  Serialize{val}.serialize(ser);
}

auto to_string(const auto& val) -> String {
  auto ser = Serializer::new_();
  Serialize{val}.serialize(ser);
  return mem::move(ser).into_string();
}

}  // namespace sfc::serde::json
