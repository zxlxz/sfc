#include "sfc/serde/json.h"

namespace sfc::serde::json {

auto to_str(Error e) -> Str {
  switch (e) {
    case Error::Success:             return "json::Error::Success";
    case Error::Finished:            return "json::Error::Finished";
    case Error::EofWhileParsing:     return "json::Error::EofWhileParsing";
    case Error::ExpectedComma:       return "json::Error::ExpectedComma";
    case Error::ExpectedDoubleQuote: return "json::Error::ExpectedDoubleQuote";
    case Error::ExpectedColon:       return "json::Error::ExpectedColon";
    case Error::ExpectedArrayBegin:  return "json::Error::ExpectedArrayBegin";
    case Error::ExpectedArrayEnd:    return "json::Error::ExpectedArrayEnd";
    case Error::ExpectedObjectBegin: return "json::Error::ExpectedObjectBegin";
    case Error::ExpectedObjectEnd:   return "json::Error::ExpectedObjectEnd";
    case Error::InvalidKeyword:      return "json::Error::InvalidKeyword";
    case Error::InvalidNumber:       return "json::Error::InvalidNumber";
    case Error::InvalidString:       return "json::Error::InvalidString";
  }
  return "json::Error::Unknown";
}

void Serializer::serialize_null() {
  _out.write_str("null");
}

void Serializer::serialize_bool(bool val) {
  _out.write_str(val ? Str{"true"} : Str{"false"});
}

void Serializer::serialize_i64(i64 val) {
  fmt::Formatter{_out}.write_val(val);
}

void Serializer::serialize_u64(u64 val) {
  fmt::Formatter{_out}.write_val(val);
}

void Serializer::serialize_f64(f64 val) {
  fmt::Formatter{_out}.write_val(val);
}

void Serializer::serialize_str(Str val) {
  _out.write_str("\"");
  _out.write_str(val);
  _out.write_str("\"");
}

auto Serializer::serialize_seq() -> SerializeSeq {
  return SerializeSeq{*this};
}

auto Serializer::serialize_obj() -> SerializeObj {
  return SerializeObj{*this};
}

SerializeSeq::SerializeSeq(Serializer& ser) : _ser{ser} {
  _ser._out.write_str("[");
}

SerializeSeq::~SerializeSeq() {
  _ser._out.write_str("]");
}

void SerializeSeq::serialize_imp() {
  if (_count++ != 0) {
    _ser._out.write_str(",");
  }
}

SerializeObj::SerializeObj(Serializer& ser) : _ser{ser} {
  _ser._out.write_str("{");
}

SerializeObj::~SerializeObj() {
  _ser._out.write_str("}");
}

void SerializeObj::serialize_key(Str val) {
  if (_count++ != 0) {
    _ser._out.write_str(",");
  }
  _ser._out.write_str("\"");
  _ser._out.write_str(val);
  _ser._out.write_str("\":");
}

Deserializer::Deserializer(Str s) : _buf{s} {}

Deserializer::~Deserializer() {}

void Deserializer::consume(usize cnt) {
  _buf = _buf[{cnt, _buf._len}];
}

auto Deserializer::peek_tok() -> Token {
  _buf = _buf.trim_start();

  const auto ch = _buf[0];
  switch (ch) {
    case 0:   return Token::Eof;
    case ',': return Token::Comma;
    case ':': return Token::Colon;
    case '"': return Token::DoubleQuote;
    case '[': return Token::ArrayBegin;
    case ']': return Token::ArrayEnd;
    case '{': return Token::ObjectBegin;
    case '}': return Token::ObjectEnd;
    case 'n': return Token::Null;
    case 't': return Token::True;
    case 'f': return Token::False;
    default:  return Token::Other;
  }
}

auto Deserializer::pop_tok(Token tok) -> Result<> {
  const auto next_tok = this->peek_tok();
  if (next_tok != tok) {
    switch (tok) {
      case Token::Comma:       return Error::ExpectedComma;
      case Token::DoubleQuote: return Error::ExpectedDoubleQuote;
      case Token::Colon:       return Error::ExpectedColon;
      case Token::ArrayBegin:  return Error::ExpectedArrayBegin;
      case Token::ArrayEnd:    return Error::ExpectedArrayEnd;
      case Token::ObjectBegin: return Error::ExpectedObjectBegin;
      case Token::ObjectEnd:   return Error::ExpectedObjectEnd;
      default:                 return Error::InvalidKeyword;
    }
  }
  this->consume(1);
  return Ok{};
}

auto Deserializer::deserialize_null() -> Result<> {
  if (!_buf.starts_with("null")) {
    return Error::InvalidKeyword;
  }
  this->consume(4);
  return Ok{};
}

auto Deserializer::deserialize_bool() -> Result<bool> {
  const auto tok = this->peek_tok();
  switch (tok) {
    case Token::True:  this->consume(4); return {true};
    case Token::False: this->consume(5); return {false};
    case Token::Eof:   return Error::EofWhileParsing;
    default:           return Error::InvalidKeyword;
  }
}

auto Deserializer::deserialize_str() -> Result<Str> {
  const auto tok = this->peek_tok();
  if (tok != Token::DoubleQuote) {
    return Error::ExpectedDoubleQuote;
  }

  this->consume(1);  // consume '"'
  const auto pos = _buf.find('"').unwrap_or(_buf._len);
  if (pos == _buf._len) {
    return Error::InvalidString;
  }

  const auto res = _buf[{0, pos}];
  this->consume(res.len());
  this->consume(1);  // consume '"'
  return Ok{res};
}

auto Deserializer::deserialize_num() -> Result<Str> {
  const auto is_spliter = [](char c) { return c == 0 || c == ',' || c == ']' || c == '}'; };

  const auto pos = _buf.find(is_spliter).unwrap_or(_buf.len());
  if (pos == 0) {
    return Error::InvalidNumber;
  }

  const auto num = _buf[{0, pos}];
  this->consume(pos);
  return num;
}

auto Deserializer::deserialize_i64() -> Result<i64> {
  const auto num_str = _TRY(this->deserialize_num());
  const auto num_ret = num_str.template parse<i64>().ok_or(Error::InvalidNumber);
  return num_ret;
}

auto Deserializer::deserialize_u64() -> Result<u64> {
  const auto num_str = _TRY(this->deserialize_num());
  const auto num_ret = num_str.template parse<u64>().ok_or(Error::InvalidNumber);
  return num_ret;
}

auto Deserializer::deserialize_f64() -> Result<f64> {
  const auto num_str = _TRY(this->deserialize_num());
  const auto num_ret = num_str.template parse<f64>().ok_or(Error::InvalidNumber);
  return num_ret;
}

DeserializeSeq::DeserializeSeq(Deserializer& inn) : _des{inn} {}

DeserializeSeq::~DeserializeSeq() {}

auto DeserializeSeq::next_imp() -> Result<bool> {
  if (_finished) {
    return false;
  }

  const auto tok = _des.peek_tok();
  if (tok == Token::ArrayEnd) {  // ']'
    _finished = true;
    return false;
  }

  if (_count != 0) {
    _TRY(_des.pop_tok(Token::Comma));
  }

  _count += 1;
  return true;
}

DeserializeObj::DeserializeObj(Deserializer& inn) : _des{inn} {}

DeserializeObj::~DeserializeObj() {}

auto DeserializeObj::next_key() -> Result<Option<Str>> {
  if (_finished) {
    return Option<Str>{};
  }

  const auto tok = _des.peek_tok();
  if (tok == Token::ObjectEnd) {  // '}'
    _finished = true;
    return Option<Str>{};
  }

  if (_count != 0) {
    _TRY(_des.pop_tok(Token::Comma));
  }

  _count += 1;

  const auto key = _TRY(_des.deserialize_str());
  _TRY(_des.pop_tok(Token::Colon));

  return Option<Str>{key};
}

}  // namespace sfc::serde::json
