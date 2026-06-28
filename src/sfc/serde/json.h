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

template <class T = void>
using Result = result::Result<T, Error>;

class Serializer {
  String& _buf;

 public:
  explicit Serializer(String& buf) : _buf{buf} {}

  void serialize_null();
  void serialize_bool(bool val);
  void serialize_i64(i64 val);
  void serialize_u64(u64 val);
  void serialize_f64(f64 val);
  void serialize_str(Str val);

  class SerializeSeq;
  auto serialize_seq() -> SerializeSeq;

  class SerializeObj;
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

class Serializer::SerializeSeq {
  Serializer& _ser;
  usize _count = 0;
  void serialize_next();

 public:
  explicit SerializeSeq(Serializer& ser);
  ~SerializeSeq();
  SerializeSeq(const SerializeSeq&) = delete;
  void operator=(const SerializeSeq&) = delete;

  void serialize_element(const auto& item) {
    this->serialize_next();
    _ser.serialize_any(item);
  }
};

class Serializer::SerializeObj {
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

  auto next_token() -> Token;
  void consume(usize cnt);

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

  class DeserializeSeq;
  template <class V, class U = FnOut<V, DeserializeSeq&>>
  auto deserialize_seq(V&& visit) -> U;

  class DeserializeObj;
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

class Deserializer::DeserializeSeq {
  using Error = json::Error;
  Deserializer& _des;
  usize _count{0};
  bool _finished{false};

  auto next_imp() -> Result<>;

 public:
  DeserializeSeq(Deserializer& inn);
  ~DeserializeSeq();

  template <class T>
  auto next_element() -> Result<T> {
    _TRY(this->next_imp());
    return _des.deserialize_any<T>();
  }

  template <class Seq, class T>
  auto collect() -> Result<Seq> {
    auto seq = Seq{};
    while (true) {
      auto elmt = this->next_element<T>();
      if (_finished) break;
      _TRY(elmt);
      seq.push(mem::move(elmt).unwrap());
    }
    return {seq};
  }
};

class Deserializer::DeserializeObj {
  using Error = json::Error;
  Deserializer& _des;
  usize _count{0};
  bool _finished{false};

  auto next_imp() -> Result<>;

 public:
  DeserializeObj(Deserializer& inn);
  ~DeserializeObj();

  auto next_key() -> Result<Str>;

  template <class T>
  auto next_value() -> Result<T> {
    return _des.deserialize_any<T>();
  }

  template <class T>
  auto next_entry() -> Result<Tuple<Str, T>> {
    auto key = _TRY(this->next_key());
    auto val = _TRY(this->next_value<T>());
    return {Tuple{mem::move(key), mem::move(val)}};
  }

  template <class Obj, class K, class V>
  auto collect() -> Result<Obj> {
    auto obj = Obj{};
    while (true) {
      const auto key = this->next_key();
      if (_finished) break;
      _TRY(key);
      auto val = _TRY(this->next_value<V>());
      obj.insert(key, mem::move(val));
    }
    return {obj};
  }
};

template <class V, class U>
auto Deserializer::deserialize_seq(V&& visit) -> U {
  auto imp = DeserializeSeq{*this};
  auto res = visit(imp);
  if (res.is_err()) return res;
  return res;
}

template <class V, class U>
auto Deserializer::deserialize_obj(V&& visit) -> U {
  auto imp = DeserializeObj{*this};
  auto res = visit(imp);
  if (res.is_err()) return res;
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
