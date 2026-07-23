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
  Finished,             // finished parsing
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
auto to_str(Error err) -> Str;

template <class T = Unit>
using Result = result::Result<T, Error>;

class Serializer;
class SerializeSeq;
class SerializeObj;

class Deserializer;
class DeserializeSeq;
class DeserializeObj;

class Serializer {
  friend class SerializeSeq;
  friend class SerializeObj;
  fmt::DynWrite _out;

 public:
  explicit Serializer(auto& out) : _out{out} {}

 public:
  void serialize_null();
  void serialize_bool(bool val);
  void serialize_i64(i64 val);
  void serialize_u64(u64 val);
  void serialize_f64(f64 val);
  void serialize_str(Str val);

  auto serialize_seq() -> SerializeSeq;
  auto serialize_obj() -> SerializeObj;

  template <class T>
  void serialize_any(const T& val) {
    if constexpr (requires { val.serialize(*this); }) {
      return val.serialize(*this);
    } else if constexpr (trait::same_<T, bool>) {
      return this->serialize_bool(val);
    } else if constexpr (trait::sint_<T>) {
      return this->serialize_i64(val);
    } else if constexpr (trait::uint_<T>) {
      return this->serialize_u64(val);
    } else if constexpr (trait::float_<T>) {
      return this->serialize_f64(val);
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
  friend class DeserializeSeq;
  friend class DeserializeObj;
  Str _buf;

  void consume(usize cnt);
  auto peek_tok() -> Token;
  auto pop_tok(Token tok) -> Result<>;

 public:
  Deserializer(Str s);
  ~Deserializer();

  auto deserialize_null() -> Result<>;
  auto deserialize_bool() -> Result<bool>;
  auto deserialize_str() -> Result<Str>;
  auto deserialize_u64() -> Result<u64>;
  auto deserialize_i64() -> Result<i64>;
  auto deserialize_f64() -> Result<f64>;
  auto deserialize_num() -> Result<Str>;

  template <class V, class U = FnOut<V, DeserializeSeq&>>
  auto deserialize_seq(V&& visit) -> U;

  template <class V, class U = FnOut<V, DeserializeObj&>>
  auto deserialize_obj(V&& visit) -> U;

  template <class T>
  auto deserialize_any() -> Result<T> {
    if constexpr (requires { T::deserialize(*this); }) {
      return T::deserialize(*this);
    } else if constexpr (trait::same_<T, bool>) {
      return this->deserialize_bool();
    } else if constexpr (trait::sint_<T>) {
      return this->deserialize_i64().map([](i64 v) { return num::saturating_cast<T>(v); });
    } else if constexpr (trait::uint_<T>) {
      return this->deserialize_u64().map([](u64 v) { return num::saturating_cast<T>(v); });
    } else if constexpr (trait::float_<T>) {
      return this->deserialize_f64().map([](f64 v) { return T(v); });
    } else if constexpr (requires { T{Str{}}; }) {
      return this->deserialize_str().and_then([](Str s) { return T{s}; });
    } else {
      static_assert(false, "json::Deserializer::deserialize: not deserializable");
    }
  }
};

class DeserializeSeq {
  using Error = json::Error;
  Deserializer& _des;
  usize _count{0};
  bool _finished{false};

 public:
  DeserializeSeq(Deserializer& inn);
  ~DeserializeSeq();

 public:
  auto next_imp() -> Result<bool>;

  template <class T>
  auto next_element() -> Result<Option<T>> {
    const auto has_next = _TRY(this->next_imp());
    if (!has_next) {
      return Option<T>{};
    }
    auto element = _TRY(_des.deserialize_any<T>());
    return {mem::move(element)};
  }

  template <class Seq, class T>
  auto collect() -> Result<Seq> {
    auto seq = Seq{};
    while (true) {
      auto opt = _TRY(this->next_element<T>());
      if (!opt) {
        break;
      }
      seq.push(mem::move(opt).unwrap());
    }
    return {seq};
  }
};

class DeserializeObj {
  using Error = json::Error;
  Deserializer& _des;
  usize _count{0};
  bool _finished{false};

 public:
  DeserializeObj(Deserializer& inn);
  ~DeserializeObj();

  auto next_key() -> Result<Option<Str>>;

  template <class T>
  auto next_val() -> Result<T> {
    return _des.deserialize_any<T>();
  }

  template <class Obj, class K, class V>
  auto collect() -> Result<Obj> {
    auto obj = Obj{};
    while (true) {
      auto key = _TRY(this->next_key());
      if (!key) {
        break;
      }
      auto val = _TRY(this->next_val<V>());
      obj.insert(key, mem::move(val));
    }
    return {obj};
  }
};

template <class V, class U>
auto Deserializer::deserialize_seq(V&& visit) -> U {
  _TRY(this->pop_tok(Token::ArrayBegin));
  auto imp = DeserializeSeq{*this};
  auto res = visit(imp);
  _TRY(this->pop_tok(Token::ArrayEnd));
  return res;
}

template <class V, class U>
auto Deserializer::deserialize_obj(V&& visit) -> U {
  _TRY(this->pop_tok(Token::ObjectBegin));
  auto imp = DeserializeObj{*this};
  auto res = visit(imp);
  _TRY(this->pop_tok(Token::ObjectEnd));
  return res;
}

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
