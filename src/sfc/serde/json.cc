#include "sfc/serde/json.h"

namespace sfc::serde::json {

void Serializer::serialize_null() noexcept {
  _buf.push_str("null");
}

void Serializer::serialize_bool(bool val) noexcept {
  _buf.push_str(val ? Str{"true"} : Str{"false"});
}

void Serializer::serialize_i64(i64 val) noexcept {
  char buf[32];
  const auto s = fmt::Debug::format_int(buf, val);
  _buf.push_str(s);
}

void Serializer::serialize_u64(u64 val) noexcept {
  char buf[32];
  const auto s = fmt::Debug::format_int(buf, val);
  _buf.push_str(s);
}

void Serializer::serialize_f64(f64 val) noexcept {
  char buf[32];
  const auto s = fmt::Debug::format_flt(buf, val, 6);
  _buf.push_str(s);
}

void Serializer::serialize_str(Str val) noexcept {
  _buf.push('"');
  _buf.push_str(val);
  _buf.push('"');
}

void Serializer::serialize_key(Str val) noexcept {
  _buf.push('"');
  _buf.push_str(val);
  _buf.push('"');
  _buf.push(':');
}

void Serializer::write_comma() noexcept {
  _buf.push(',');
}

auto Serializer::serialize_seq() noexcept -> SerArray {
  return SerArray{*this};
}

auto Serializer::serialize_obj() noexcept -> SerObject {
  return SerObject{*this};
}

auto Serializer::serialize_map() noexcept -> SerObject {
  return SerObject{*this};
}

Serializer::SerArray::SerArray(Serializer& ser) : _ser{ser} {
  _ser._buf.push('[');
}

Serializer::SerArray::~SerArray() {
  _ser._buf.push(']');
}

Serializer::SerObject::SerObject(Serializer& ser) : _ser{ser} {
  _ser._buf.push('{');
}

Serializer::SerObject::~SerObject() {
  _ser._buf.push('}');
}

auto Deserializer::deserialize_null() noexcept -> Result<> {
  return this->extract_tok(Token::Null);
}

auto Deserializer::deserialize_bool() noexcept -> Result<bool> {
  const auto next_tok = this->next_token();
  switch (next_tok) {
    case Token::True:  this->consume(4); return Ok{true};
    case Token::False: this->consume(5); return Ok{false};
    case Token::Eof:   return Err{Error::EofWhileParsing};
    default:           return Err{Error::InvalidKeyword};
  }
}

auto Deserializer::deserialize_str() noexcept -> Result<Str> {
  const auto next_tok = this->next_token();
  if (next_tok != Token::DoubleQuote) {
    return Err{Error::ExpectedDoubleQuote};
  }

  const auto s = _buf[{1, _buf._len}];
  const auto p = s.find('"').unwrap_or(s._len);
  if (p == s._len) {
    return Err{Error::InvalidString};
  }

  this->consume(p + 2);
  return Ok{s[{0, p}]};
}

auto Deserializer::deserialize_num() noexcept -> Result<Str> {
  _buf = _buf.trim_start();

  const auto not_digits = [](char c) {
    return !(('0' <= c && c <= '9') || c == '+' || c == '-' || c == '.' || c == 'e' || c == 'E');
  };
  const auto pos = _buf.find(not_digits).unwrap_or(_buf.len());
  if (pos == 0) {
    return Err{Error::InvalidNumber};
  }

  const auto num = _buf[{0, pos}];
  this->consume(pos);
  return Ok{num};
}

auto Deserializer::deserialize_i64() noexcept -> Result<i64> {
  const auto num_str = _TRY(this->deserialize_num());
  const auto num_val = num_str.template parse<i64>();
  if (!num_val) {
    return Err{Error::InvalidNumber};
  }
  return Ok{*num_val};
}

auto Deserializer::deserialize_u64() noexcept -> Result<u64> {
  const auto num_str = _TRY(this->deserialize_num());
  const auto num_val = num_str.template parse<u64>();
  if (!num_val) {
    return Err{Error::InvalidNumber};
  }
  return Ok{*num_val};
}

auto Deserializer::deserialize_f64() noexcept -> Result<f64> {
  const auto num_str = _TRY(this->deserialize_num());
  const auto num_val = num_str.template parse<f64>();
  if (!num_val) {
    return Err{Error::InvalidNumber};
  }
  return Ok{*num_val};
}

auto Deserializer::deserialize_key() noexcept -> Result<Str> {
  auto s = this->deserialize_str();
  if (s.is_err()) {
    return s;
  }
  _TRY(this->extract_tok(Token::Colon));
  return s;
}

void Deserializer::consume(usize n) noexcept {
  if (n > _buf._len) {
    return;
  }
  _buf._ptr += n;
  _buf._len -= n;
}

auto Deserializer::next_token() noexcept -> Token {
  _buf = _buf.trim_start();
  if (_buf.is_empty()) {
    return Token::Eof;
  }
  switch (_buf[0]) {
    case 0:   return Token::Eof;
    case ',': return Token::Comma;
    case ':': return Token::Colon;
    case '"': return Token::DoubleQuote;
    case '[': return Token::ArrayBegin;
    case ']': return Token::ArrayEnd;
    case '{': return Token::ObjectBegin;
    case '}': return Token::ObjectEnd;
    case 'n': return _buf.starts_with("null") ? Token::Null : Token::Other;
    case 't': return _buf.starts_with("true") ? Token::True : Token::Other;
    case 'f': return _buf.starts_with("false") ? Token::False : Token::Other;
    default:  return Token::Other;
  }
}

auto Deserializer::extract_tok(Token expect_tok) noexcept -> Result<> {
  const auto next_tok = this->next_token();
  if (next_tok != expect_tok) {
    switch (expect_tok) {
      case Token::Comma:       return Err{Error::ExpectedComma};
      case Token::DoubleQuote: return Err{Error::ExpectedDoubleQuote};
      case Token::Colon:       return Err{Error::ExpectedColon};
      case Token::ArrayBegin:  return Err{Error::ExpectedArrayBegin};
      case Token::ArrayEnd:    return Err{Error::ExpectedArrayEnd};
      case Token::ObjectBegin: return Err{Error::ExpectedObjectBegin};
      case Token::ObjectEnd:   return Err{Error::ExpectedObjectEnd};
      case Token::Null:        return Err{Error::InvalidKeyword};
      case Token::True:        return Err{Error::InvalidKeyword};
      case Token::False:       return Err{Error::InvalidKeyword};
      case Token::Eof:         return Err{Error::EofWhileParsing};
      default:                 return Err{Error::InvalidKeyword};
    }
  }

  switch (expect_tok) {
    case Token::Comma:       this->consume(1); break;
    case Token::Colon:       this->consume(1); break;
    case Token::DoubleQuote: this->consume(1); break;
    case Token::ArrayBegin:  this->consume(1); break;
    case Token::ArrayEnd:    this->consume(1); break;
    case Token::ObjectBegin: this->consume(1); break;
    case Token::ObjectEnd:   this->consume(1); break;
    case Token::Null:        this->consume(4); break;
    case Token::True:        this->consume(4); break;
    case Token::False:       this->consume(5); break;
    default:                 break;
  }
  return Ok{};
}

}  // namespace sfc::serde::json
