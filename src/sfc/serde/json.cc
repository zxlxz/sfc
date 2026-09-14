#include "sfc/serde/json.h"

namespace sfc::serde::json {

namespace detail {

static auto is_blank(u8 ch) -> bool {
  return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
}

static auto is_num_char(u8 ch) -> bool {
  return (ch >= '0' && ch <= '9') || ch == '-' || ch == '+' || ch == '.';
}

}  // namespace detail

auto to_str(Error e) -> Str {
  switch (e) {
    case Error::Success:             return "json::Error::Success";
    case Error::Eof:                 return "json::Error::Eof";
    case Error::IOError:             return "json::Error::IOError";
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

Serializer::Serializer(io::DynWrite out) : _out{out} {}

void Serializer::write_str(Str s) {
  (void)_out.write_str(s);
}

void Serializer::serialize_null() {
  this->write_str("null");
}

void Serializer::serialize_bool(bool val) {
  this->write_str(val ? Str{"true"} : Str{"false"});
}

void Serializer::serialize_i64(i64 val) {
  fmt::Formatter(this->_out).write_val(val);
}

void Serializer::serialize_u64(u64 val) {
  fmt::Formatter(this->_out).write_val(val);
}

void Serializer::serialize_f64(f64 val) {
  fmt::Formatter(this->_out).write_val(val);
}

void Serializer::serialize_str(Str val) {
  this->write_str("\"");
  this->write_str(val);
  this->write_str("\"");
}

auto Serializer::serialize_seq() -> SerializeSeq {
  return SerializeSeq{*this};
}

auto Serializer::serialize_obj() -> SerializeObj {
  return SerializeObj{*this};
}

SerializeSeq::SerializeSeq(Serializer& ser) : _ser{ser} {
  _ser.write_str("[");
}

SerializeSeq::~SerializeSeq() {
  _ser.write_str("]");
}

void SerializeSeq::serialize_imp() {
  if (_count++ != 0) {
    _ser.write_str(",");
  }
}

SerializeObj::SerializeObj(Serializer& ser) : _ser{ser} {
  _ser.write_str("{");
}

SerializeObj::~SerializeObj() {
  _ser.write_str("}");
}

void SerializeObj::serialize_key(Str val) {
  if (_count++ != 0) {
    _ser.write_str(",");
  }
  _ser.write_str("\"");
  _ser.write_str(val);
  _ser.write_str("\":");
}

Deserializer::Deserializer(io::DynRead r) : _reader{r} {}

auto Deserializer::read_chr() -> Result<u8> {
  _peek_char = ' ';

  u8 buf[1] = {0};

  const auto ret = _reader.read({buf, 1});
  if (ret.is_err()) {
    return Err(Error::IOError);
  }

  const auto cnt = ret.as_ok().unwrap_or(0);
  if (cnt == 0) {
    return Err(Error::Eof);
  }

  return Ok(buf[0]);
}

auto Deserializer::peek_tok() -> Result<Token> {
  // skip-blank
  while (detail::is_blank(_peek_char)) {
    _peek_char = _TRY(this->read_chr());
  }

  switch (_peek_char) {
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
    case '+':
    case '-':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': return Token::Number;
    default:  return Err(Error::InvalidKeyword);
  }
}

auto Deserializer::read_tok(Token expected) -> Result<Token> {
  const auto tok = _TRY(this->peek_tok());
  if (expected != Token::Unknown && tok != expected) {
    switch (expected) {
      case Token::Comma:       return Err(Error::ExpectedComma);
      case Token::Colon:       return Err(Error::ExpectedColon);
      case Token::ArrayBegin:  return Err(Error::ExpectedArrayBegin);
      case Token::ArrayEnd:    return Err(Error::ExpectedArrayEnd);
      case Token::ObjectBegin: return Err(Error::ExpectedObjectBegin);
      case Token::ObjectEnd:   return Err(Error::ExpectedObjectEnd);
      case Token::DoubleQuote: return Err(Error::ExpectedDoubleQuote);
      default:                 return Err(Error::InvalidKeyword);
    }
  }

  auto read_out = [&]<u32 N>(const char (&s)[N]) -> Result<> {
    _peek_char = ' ';
    for (u32 i = 1; i < N - 1; ++i) {
      const auto ch = _TRY(this->read_chr());
      if (ch != s[i]) {
        return Err(Error::InvalidKeyword);
      }
    }
    return Ok{};
  };

  switch (tok) {
    case Token::Null:        _TRY(read_out("null")); break;
    case Token::True:        _TRY(read_out("true")); break;
    case Token::False:       _TRY(read_out("false")); break;
    case Token::Comma:       _TRY(read_out(",")); break;
    case Token::Colon:       _TRY(read_out(":")); break;
    case Token::ArrayBegin:  _TRY(read_out("[")); break;
    case Token::ArrayEnd:    _TRY(read_out("]")); break;
    case Token::ObjectBegin: _TRY(read_out("{")); break;
    case Token::ObjectEnd:   _TRY(read_out("}")); break;
    case Token::DoubleQuote: break;
    case Token::Number:      break;
    case Token::Unknown:     break;
  }

  return tok;
}

auto Deserializer::read_str() -> Result<String> {
  _TRY(this->read_tok());  // eat '"'

  auto res = String{};
  auto prev_char = u8(0);
  while (true) {
    const auto ch = _TRY(this->read_chr());
    if (prev_char == '\\') {
      switch (ch) {
        case '"':  res.push('"'); break;
        case '\\': res.push('\\'); break;
        case 'n':  res.push('\n'); break;
        case 'r':  res.push('\r'); break;
        case 't':  res.push('\t'); break;
        default:   return Error::InvalidString;
      }
      prev_char = 0;
      continue;
    }
    if (ch == '"') {
      break;  // eat '"'
    }
    if (ch != '\\') {
      res.push(ch);
    }
    prev_char = ch;
  }
  return Ok{mem::move(res)};
}

auto Deserializer::read_num(Slice<u8> buf) -> Result<Str> {
  const auto cnt = buf.len();

  auto idx = 0U;
  if (idx < cnt) {
    buf[idx++] = _peek_char;
  }

  for (; idx < cnt;) {
    const auto ret = this->read_chr();
    if (auto err = ret.as_err()) {
      break;
    }
    const auto ch = ret.as_ok().unwrap_or(0);
    if (!detail::is_num_char(ch)) {
      _peek_char = ch;
      break;
    }
    buf[idx++] = ch;
  }
  const auto s = Str::from_utf8({buf.as_ptr(), idx});
  return Ok{s};
}

auto Deserializer::deserialize_null() -> Result<> {
  const auto tok = this->peek_tok();
  if (tok != Token::Null) {
    return Error::InvalidKeyword;
  }

  _TRY(this->read_tok());
  return Ok{};
}

auto Deserializer::deserialize_bool() -> Result<bool> {
  const auto tok = _TRY(this->peek_tok());
  if (tok != Token::True && tok != Token::False) {
    return Error::InvalidKeyword;
  }

  _TRY(this->read_tok());
  return tok == Token::True;
}

auto Deserializer::deserialize_i64() -> Result<i64> {
  if (_TRY(this->peek_tok()) != Token::Number) {
    return Error::InvalidNumber;
  }

  u8 buf[64];
  const auto num_str = _TRY(this->read_num(buf));
  const auto num_val = num_str.template parse<i64>().ok_or(Error::InvalidNumber);
  return num_val;
}

auto Deserializer::deserialize_u64() -> Result<u64> {
  if (_TRY(this->peek_tok()) != Token::Number) {
    return Error::InvalidNumber;
  }

  u8 buf[64];
  const auto num_str = _TRY(this->read_num(buf));
  const auto num_val = num_str.template parse<u64>().ok_or(Error::InvalidNumber);
  return num_val;
}

auto Deserializer::deserialize_f64() -> Result<f64> {
  if (_TRY(this->peek_tok()) != Token::Number) {
    return Error::InvalidNumber;
  }

  u8 buf[64];
  const auto num_str = _TRY(this->read_num(buf));
  const auto num_val = num_str.template parse<f64>().ok_or(Error::InvalidNumber);
  return num_val;
}

auto Deserializer::deserialize_string() -> Result<String> {
  const auto tok = _TRY(this->peek_tok());
  if (tok != Token::DoubleQuote) {
    return Error::ExpectedDoubleQuote;
  }

  return this->read_str();
}

DeserializeSeq::DeserializeSeq(Deserializer& inn) : _des{inn} {}

DeserializeSeq::~DeserializeSeq() {}

auto DeserializeSeq::next_imp() -> Result<bool> {
  if (_finished) {
    return false;
  }

  const auto tok = _TRY(_des.peek_tok());
  if (tok == Token::ArrayEnd) {
    _TRY(_des.read_tok());
    _finished = true;
    return false;
  }

  if (_count != 0) {
    if (tok != Token::Comma) {
      return Error::ExpectedComma;
    }
    _TRY(_des.read_tok());
  }

  _count += 1;
  return true;
}

DeserializeObj::DeserializeObj(Deserializer& inn) : _des{inn} {}

DeserializeObj::~DeserializeObj() {}

auto DeserializeObj::next_key() -> Result<Option<String>> {
  if (_finished) {
    return Option<String>{};
  }

  const auto tok = _TRY(_des.peek_tok());
  if (tok == Token::ObjectEnd) {
    _finished = true;
    return Option<String>{};
  }

  if (_count != 0) {
    if (tok != Token::Comma) {
      return Error::ExpectedComma;
    }
    _TRY(_des.read_tok());
  }

  _count += 1;
  auto key = _TRY(_des.deserialize_string());
  _TRY(_des.read_tok(Token::Colon));
  return Option<String>{mem::move(key)};
}

}  // namespace sfc::serde::json
