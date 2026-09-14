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

auto Deserializer::peek() -> Result<u8> {
  auto ch = u8(_peek_char ? _peek_char : ' ');

  while (detail::is_blank(ch)) {
    u8 buf[1] = {};
    if (auto ret = _reader.read({buf}); ret.is_err()) {
      return Error::IOError;
    }
    ch = buf[0];
  }
  _peek_char = ch;
  return Ok{ch};
}

auto Deserializer::peak_tok() -> Result<Token> {
  const auto ch = _TRY(this->peek());
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
    default:  return detail::is_num_char(ch) ? Token::Number : Token::Other;
  }
}

auto Deserializer::next_tok() -> Result<Token> {
  const auto tok = _TRY(this->peak_tok());
  switch (tok) {
    case Token::Null:
      _TRY(this->read_key("null"));
      return tok;
    case Token::True:
      _TRY(this->read_key("true"));
      return tok;
    case Token::False:
      _TRY(this->read_key("false"));
      return tok;
    case Token::Comma:
    case Token::Colon:
    case Token::ArrayBegin:
    case Token::ArrayEnd:
    case Token::ObjectBegin:
    case Token::ObjectEnd:
      _TRY(this->next());
      return tok;
    case Token::Eof:
    case Token::DoubleQuote:
    case Token::Number:
    case Token::Other:
      return tok;
  }
  return Error::InvalidKeyword;
}

auto Deserializer::next() -> Result<u8> {
  if (_peek_char != 0) {
    const auto ch = mem::take(_peek_char);
    return Ok{ch};
  }

  u8 buf[1] = {};
  if (auto ret = _reader.read({buf}); ret.is_err()) {
    return Error::IOError;
  }
  return Ok{buf[0]};
}

auto Deserializer::read_tok(char tok) -> Result<> {
  const auto ch = _TRY(this->peek());
  if (ch == u8(tok)) {
    _peek_char = 0;
    return Ok{};
  }

  switch (tok) {
    case ',': return Error::ExpectedComma;
    case ':': return Error::ExpectedColon;
    case '[': return Error::ExpectedArrayBegin;
    case ']': return Error::ExpectedArrayEnd;
    case '{': return Error::ExpectedObjectBegin;
    case '}': return Error::ExpectedObjectEnd;
    case '"': return Error::ExpectedDoubleQuote;
  }
  return Error::InvalidKeyword;
}

auto Deserializer::read_key(Str s) -> Result<> {
  const auto cnt = s.len();
  const auto ptr = s.as_ptr();

  for (auto idx = 0U; idx < cnt; ++idx) {
    const auto next_ch = _TRY(this->next());
    if (next_ch != ptr[idx]) {
      return Error::InvalidKeyword;
    }
  }
  return Ok{};
}

auto Deserializer::read_num(Slice<u8> buf) -> Result<Str> {
  const auto cnt = buf.len();

  auto idx = 0U;
  for (; idx < cnt;) {
    const auto ch = _TRY(this->peek());
    if (!detail::is_num_char(ch)) {
      break;
    }
    _peek_char = 0;
    buf[idx++] = ch;
  }

  if (idx == cnt && detail::is_num_char(_TRY(this->peek()))) {
    return Error::InvalidNumber;
  }

  const auto s = Str::from_utf8({buf.as_ptr(), idx});
  return Ok{s};
}

auto Deserializer::read_str() -> Result<String> {
  _TRY(this->read_tok('"'));
  auto res = String{};
  auto prev_char = u8(0);

  while (true) {
    const auto ch = _TRY(this->next());
    if (ch == 0) {
      return Error::InvalidString;
    }

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

    if (ch == '\\') {
      prev_char = ch;
      continue;
    }

    if (ch == '"') {
      break;
    }
    res.push(ch);
  }
  return Ok{mem::move(res)};
}

auto Deserializer::deserialize_null() -> Result<> {
  if (_TRY(this->next_tok()) == Token::Null) {
    return Ok{};
  }
  return Error::InvalidKeyword;
}

auto Deserializer::deserialize_bool() -> Result<bool> {
  switch (_TRY(this->next_tok())) {
    case Token::True: {
      return {true};
    }
    case Token::False: {
      return {false};
    }
    default: {
      return Error::InvalidKeyword;
    }
  }
}

auto Deserializer::deserialize_i64() -> Result<i64> {
  if (_TRY(this->peak_tok()) != Token::Number) {
    return Error::InvalidNumber;
  }

  u8 buf[64];
  const auto num_str = _TRY(this->read_num(buf));
  const auto num_val = num_str.template parse<i64>().ok_or(Error::InvalidNumber);
  return num_val;
}

auto Deserializer::deserialize_u64() -> Result<u64> {
  if (_TRY(this->peak_tok()) != Token::Number) {
    return Error::InvalidNumber;
  }

  u8 buf[64];
  const auto num_str = _TRY(this->read_num(buf));
  const auto num_val = num_str.template parse<u64>().ok_or(Error::InvalidNumber);
  return num_val;
}

auto Deserializer::deserialize_f64() -> Result<f64> {
  if (_TRY(this->peak_tok()) != Token::Number) {
    return Error::InvalidNumber;
  }

  u8 buf[64];
  const auto num_str = _TRY(this->read_num(buf));
  const auto num_val = num_str.template parse<f64>().ok_or(Error::InvalidNumber);
  return num_val;
}

auto Deserializer::deserialize_string() -> Result<String> {
  if (_TRY(this->peak_tok()) != Token::DoubleQuote) {
    return Error::ExpectedDoubleQuote;
  }

  auto str = this->read_str();
  return str;
}

DeserializeSeq::DeserializeSeq(Deserializer& inn) : _des{inn} {}

DeserializeSeq::~DeserializeSeq() {}

auto DeserializeSeq::next_imp() -> Result<bool> {
  if (_finished) {
    return false;
  }

  const auto tok = _TRY(_des.peak_tok());
  if (tok == Token::ArrayEnd) {
    _finished = true;
    return false;
  }

  if (_count != 0) {
    if (_TRY(_des.next_tok()) != Token::Comma) {
      return Error::ExpectedComma;
    }
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

  const auto tok = _TRY(_des.peak_tok());
  if (tok == Token::ObjectEnd) {
    _finished = true;
    return Option<String>{};
  }

  if (_count != 0) {
    if (_TRY(_des.next_tok()) != Token::Comma) {
      return Error::ExpectedComma;
    }
  }

  _count += 1;

  auto key = _TRY(_des.deserialize_string());
  _TRY(_des.read_tok(':'));

  return Option<String>{mem::move(key)};
}

}  // namespace sfc::serde::json
