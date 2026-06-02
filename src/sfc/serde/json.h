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

class Serializer {
  String& _buf;

 public:
  explicit Serializer(String& buf) : _buf{buf} {}

  void serialize_null() noexcept;
  void serialize_bool(bool val) noexcept;
  void serialize_i64(i64 val) noexcept;
  void serialize_u64(u64 val) noexcept;
  void serialize_f64(f64 val) noexcept;
  void serialize_str(Str val) noexcept;

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

 public:
  struct SerArray {
    Serializer& _ser;
    usize _cnt = 0;

   public:
    explicit SerArray(Serializer& ser);
    ~SerArray();
    SerArray(const SerArray&) = delete;
    void operator=(const SerArray&) = delete;

    void serialize_element(const auto& item) noexcept {
      if (_cnt++ != 0) _ser.write_comma();
      _ser.serialize_any(item);
    }
  };

  struct SerObject {
    Serializer& _ser;
    usize _cnt = 0;

   public:
    explicit SerObject(Serializer& ser);
    ~SerObject();
    SerObject(const SerObject&) = delete;
    SerObject& operator=(const SerObject&) = delete;

    void serialize_entry(Str key, const auto& val) noexcept {
      if (_cnt++ != 0) _ser.write_comma();
      _ser.serialize_key(key);
      _ser.serialize_any(val);
    }
  };

  auto serialize_seq() noexcept -> SerArray;
  auto serialize_obj() noexcept -> SerObject;
  auto serialize_map() noexcept -> SerObject;

 private:
  void write_comma() noexcept;
  void serialize_key(Str key) noexcept;
};

struct Deserializer {
  Str _buf;

 public:
  auto deserialize_null() noexcept -> Result<>;
  auto deserialize_bool() noexcept -> Result<bool>;
  auto deserialize_str() noexcept -> Result<Str>;
  auto deserialize_u64() noexcept -> Result<u64>;
  auto deserialize_i64() noexcept -> Result<i64>;
  auto deserialize_f64() noexcept -> Result<f64>;

  template <class T>
  auto deserialize_any() noexcept -> Result<T> {
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
      if (_idx++ != 0) _TRY(_inn.extract_tok(Token::Comma));
      return _inn.deserialize_any<T>();
    }

    template <class V, class T>
    auto collect() noexcept -> Result<V> {
      auto res = V{};
      while (this->has_next()) {
        auto elem = _TRY(this->next_element<T>());
        res.push(mem::move(elem));
      }
      return {res};
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

    auto next_key() noexcept -> Result<Str> {
      if (_idx++ != 0) _TRY(_inn.extract_tok(Token::Comma));
      return _inn.deserialize_key();
    }

    template <class T>
    auto next_value() noexcept -> Result<T> {
      return _inn.deserialize_any<T>();
    }

    template <class M, class K, class V>
    auto collect() noexcept -> Result<M> {
      auto res = M{};
      while (this->has_next()) {
        auto key = _TRY(this->next_key());
        auto val = _TRY(this->next_value<V>());
        res.insert(key, mem::move(val));
      }
      return {res};
    }
  };

  template <class V, class U = FnOut<V, DesArray&>>
  auto deserialize_seq(V&& visit) -> U {
    _TRY(this->extract_tok(Token::ArrayBegin));
    auto imp = DesArray{*this};
    auto res = visit(imp);
    if (res.is_err()) return res;
    _TRY(this->extract_tok(Token::ArrayEnd));
    return res;
  }

  template <class V, class U = FnOut<V, DesObject&>>
  auto deserialize_obj(V&& visit) -> U {
    _TRY(this->extract_tok(Token::ObjectBegin));
    auto imp = DesObject{*this};
    auto res = visit(imp);
    if (res.is_err()) return res;
    _TRY(this->extract_tok(Token::ObjectEnd));
    return res;
  }

  template <class V, class U = FnOut<V, DesObject&>>
  auto deserialize_map(V&& visit) -> U {
    _TRY(this->extract_tok(Token::ObjectBegin));
    auto imp = DesObject{*this};
    auto res = visit(imp);
    _TRY(this->extract_tok(Token::ObjectEnd));
    return res;
  }

 private:
  void consume(usize n) noexcept;
  auto next_token() noexcept -> Token;
  auto extract_tok(Token tok) noexcept -> Result<>;
  auto deserialize_key() noexcept -> Result<Str>;
  auto deserialize_num() noexcept -> Result<Str>;
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
