#include "sfc/serde/json.h"

namespace sfc::serde::json {

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

auto Deserializer::extract_num() noexcept -> Result<Str> {
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

auto Deserializer::extract_str() noexcept -> Result<Str> {
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

}  // namespace sfc::serde::json
