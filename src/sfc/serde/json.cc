#include "sfc/serde/json.h"

namespace sfc::serde::json {

void Serializer::write_tok(Token tok) noexcept {
  switch (tok) {
    case Token::Eof:         break;
    case Token::Comma:       _buf.push(','); break;
    case Token::Colon:       _buf.push(':'); break;
    case Token::DoubleQuote: _buf.push('"'); break;
    case Token::ArrayBegin:  _buf.push('['); break;
    case Token::ArrayEnd:    _buf.push(']'); break;
    case Token::ObjectBegin: _buf.push('{'); break;
    case Token::ObjectEnd:   _buf.push('}'); break;
    case Token::Null:        _buf.push_str("null"); break;
    case Token::True:        _buf.push_str("true"); break;
    case Token::False:       _buf.push_str("false"); break;
    case Token::Other:       break;
    default:                 break;
  }
}

void Serializer::write_str(Str s) noexcept {
  _buf.push('"');
  _buf.push_str(s);
  _buf.push('"');
}

void Serializer::write_num(Str s) noexcept {
  _buf.push_str(s);
}

void Serializer::write_key(Str s) noexcept {
  _buf.push('"');
  _buf.push_str(s);
  _buf.push_str("\":");
}

auto Serializer::serialize_seq() noexcept -> SerArray {
  return SerArray{*this};
}

auto Serializer::serialize_map() noexcept -> SerObject {
  return SerObject{*this};
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

auto Deserializer::extract_tok(Token expect_tok) noexcept -> Result<Token> {
  const auto next_tok = this->next_token();
  if (next_tok == expect_tok) {
    return next_tok;
  }

  switch (expect_tok) {
    case Token::Comma:       return Error::ExpectedComma;
    case Token::DoubleQuote: return Error::ExpectedDoubleQuote;
    case Token::Colon:       return Error::ExpectedColon;
    case Token::ArrayBegin:  return Error::ExpectedArrayBegin;
    case Token::ArrayEnd:    return Error::ExpectedArrayEnd;
    case Token::ObjectBegin: return Error::ExpectedObjectBegin;
    case Token::ObjectEnd:   return Error::ExpectedObjectEnd;
    case Token::Null:        return Error::InvalidKeyword;
    case Token::True:        return Error::InvalidKeyword;
    case Token::False:       return Error::InvalidKeyword;
    case Token::Eof:         return Error::EofWhileParsing;
    default:                 return Error::InvalidKeyword;
  }
}

auto Deserializer::extract_str() noexcept -> Result<Str> {
  const auto next_tok = this->extract_tok(Token::DoubleQuote);
  if (next_tok.is_err()) {
    return Error::ExpectedDoubleQuote;
  }

  const auto not_double_quote = [](char c) { return c != '"'; };
  const auto pos = _buf.find(not_double_quote);
  if (!pos) {
    return Error::EofWhileParsing;
  }

  const auto str = _buf[{0, *pos}];
  this->consume(*pos + 1);
  return str;
}

auto Deserializer::extract_num() noexcept -> Result<Str> {
  _buf = _buf.trim_start();

  const auto not_digits = [](char c) {
    return !(('0' <= c && c <= '9') || c == '+' || c == '-' || c == '.' || c == 'e' || c == 'E');
  };
  const auto pos = _buf.find(not_digits).unwrap_or(_buf.len());
  if (pos == 0) {
    return Error::InvalidNumber;
  }

  const auto num = _buf[{0, pos}];
  this->consume(pos);
  return num;
}

}  // namespace sfc::serde::json
