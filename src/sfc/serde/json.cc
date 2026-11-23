#include "sfc/serde/json.h"

namespace sfc::serde::json {

auto decode_string(Str s) -> String {
  auto buf = String::with_capacity(s._len);

  auto prev_char = ' ';
  for (auto c : s.as_bytes()) {
    if (prev_char == '\\') {
      switch (c) {
        case '"':  buf.push('"'); break;
        case '\\': buf.push('\\'); break;
        case 'n':  buf.push('\n'); break;
        case 'r':  buf.push('\r'); break;
        case 't':  buf.push('\t'); break;
        default:   buf.push(c); break;
      }
      prev_char = ' ';
    } else if (c == '\\') {
      prev_char = '\\';
    } else {
      buf.push(c);
    }
  }
  return buf;
};

auto to_str(Error e) -> Str {
  switch (e) {
    case Error::Success:             return "json::Error::Success";
    case Error::InvalidToken:        return "json::Error::InvalidToken";
    case Error::UnexpectedEof:       return "json::Error::UnexpectedEof";
    case Error::ExpectedComma:       return "json::Error::ExpectedComma";
    case Error::ExpectedColon:       return "json::Error::ExpectedColon";
    case Error::ExpectedArrayBegin:  return "json::Error::ExpectedArrayBegin";
    case Error::ExpectedArrayEnd:    return "json::Error::ExpectedArrayEnd";
    case Error::ExpectedObjectBegin: return "json::Error::ExpectedObjectBegin";
    case Error::ExpectedObjectEnd:   return "json::Error::ExpectedObjectEnd";
    case Error::InvalidNumber:       return "json::Error::InvalidNumber";
    case Error::InvalidString:       return "json::Error::InvalidString";
  }
  return "json::Error::Unknown";
}

Serializer::Serializer(String buf) : _buf{mem::move(buf)} {}

Serializer::~Serializer() {}

Serializer::Serializer(Serializer&&) noexcept = default;

Serializer& Serializer::operator=(Serializer&&) noexcept = default;

auto Serializer::new_() -> Serializer {
  return Serializer{String::new_()};
}

auto Serializer::as_str() const -> Str {
  return _buf.as_str();
}

auto Serializer::into_string() && -> String {
  return mem::move(_buf);
}

void Serializer::write_tok(Token tok) {
  switch (tok) {
    case Token::Comma:       _buf.write_str(", "); break;
    case Token::Colon:       _buf.write_str(": "); break;
    case Token::ArrayBegin:  _buf.write_str("["); break;
    case Token::ArrayEnd:    _buf.write_str("]"); break;
    case Token::ObjectBegin: _buf.write_str("{"); break;
    case Token::ObjectEnd:   _buf.write_str("}"); break;
    case Token::Null:        _buf.write_str("null"); break;
    case Token::True:        _buf.write_str("true"); break;
    case Token::False:       _buf.write_str("false"); break;
    default:                 break;
  }
}

void Serializer::serialize_null() {
  this->write_tok(Token::Null);
}

void Serializer::serialize_bool(bool val) {
  const auto tok = val ? Token::True : Token::False;
  this->write_tok(tok);
}

void Serializer::serialize_int(auto val) {
  fmt::write(_buf, "{}", val);
}

void Serializer::serialize_flt(auto val) {
  fmt::write(_buf, "{}", val);
}

template void Serializer::serialize_int(signed char);
template void Serializer::serialize_int(short);
template void Serializer::serialize_int(int);
template void Serializer::serialize_int(long);
template void Serializer::serialize_int(long long);

template void Serializer::serialize_int(unsigned char);
template void Serializer::serialize_int(unsigned short);
template void Serializer::serialize_int(unsigned int);
template void Serializer::serialize_int(unsigned long);
template void Serializer::serialize_int(unsigned long long);

template void Serializer::serialize_flt(float);
template void Serializer::serialize_flt(double);

void Serializer::serialize_str(Str val) {
  fmt::write(_buf, "\"{}\"", val);
}

auto Serializer::serialize_seq() -> SerializeSeq {
  return SerializeSeq{*this};
}

auto Serializer::serialize_obj() -> SerializeObj {
  return SerializeObj{*this};
}

SerializeSeq::SerializeSeq(Serializer& ser) : _ser{ser} {
  _ser.write_tok(Token::ArrayBegin);
}

SerializeSeq::~SerializeSeq() {
  _ser.write_tok(Token::ArrayEnd);
}

void SerializeSeq::serialize_imp() {
  if (_count++ != 0) {
    _ser.write_tok(Token::Comma);
  }
}

SerializeObj::SerializeObj(Serializer& ser) : _ser{ser} {
  _ser.write_tok(Token::ObjectBegin);
}

SerializeObj::~SerializeObj() {
  _ser.write_tok(Token::ObjectEnd);
}

void SerializeObj::serialize_key(Str key) {
  if (_count++ != 0) {
    _ser.write_tok(Token::Comma);
  }
  _ser.serialize_str(key);
  _ser.write_tok(Token::Colon);
}

auto Deserializer::from_str(Str buf) -> Deserializer {
  auto res = Deserializer{};
  res._buf = buf;
  return res;
}

auto Deserializer::peek_tok() -> Result<Token> {
  _buf = _buf.trim_start();

  if (_buf.is_empty()) {
    return Error::UnexpectedEof;
  }

  const auto h = _buf[0];
  switch (h) {
    case ',': return Token::Comma;
    case ':': return Token::Colon;
    case '[': return Token::ArrayBegin;
    case ']': return Token::ArrayEnd;
    case '{': return Token::ObjectBegin;
    case '}': return Token::ObjectEnd;
    case '"': return Token::String;
    case '+':
    case '-':
    case '.':
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
    case 'n': return Token::Null;
    case 't': return Token::True;
    case 'f': return Token::False;
    default:  break;
  }
  return Error::InvalidToken;
}

auto Deserializer::next_tok() -> Result<Str> {
  if (_buf.is_empty()) {
    return Error::UnexpectedEof;
  }

  auto read = [&](usize n) -> Result<Str> {
    if (_buf.len() < n) return Error::UnexpectedEof;
    const auto [a, b] = _buf.split_at(n);
    _buf = b;
    return a;
  };

  auto string_len = [&] -> usize {
    if (_buf.len() < 2) {
      return 0;
    }

    auto prev_escape = false;
    for (auto idx = 1U; idx < _buf._len; ++idx) {
      const auto c = _buf[idx];
      if (c == '"' && !prev_escape) {
        return idx + 1;
      }
      prev_escape = (c == '\\') ? !prev_escape : false;
    }
    return 0;
  };

  auto number_len = [&] -> usize {
    for (auto idx = 1U; idx < _buf._len; ++idx) {
      const auto c = _buf[idx];
      const auto is_num = ('0' <= c && c <= '9') || c == '+' || c == '-' || c == '.';
      if (!is_num) {
        return idx;
      }
    }
    return _buf._len;
  };

  switch (_buf[0]) {
    default: {
      return Error::InvalidToken;
    }
    case '[':
    case ']':
    case '{':
    case '}':
    case ',':
    case ':': {
      return read(1);
    }
    case 'n': {
      if (!_buf.starts_with("null")) return Error::InvalidToken;
      return read(4);
    }
    case 't': {
      if (!_buf.starts_with("true")) return Error::InvalidToken;
      return read(4);
    }
    case 'f': {
      if (!_buf.starts_with("false")) return Error::InvalidToken;
      return read(5);
    }
    case '"': {
      if (auto len = string_len(); len != 0) {
        return read(len);
      }
      return Error::InvalidString;
    }
    case '+':
    case '-':
    case '.':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
      if (auto len = number_len(); len != 0) {
        return read(len);
      }
      return Error::InvalidNumber;
    }
  }
}

auto Deserializer::read_tok(Token expected) -> Result<> {
  const auto next_tok = _TRY(this->peek_tok());
  if (next_tok != expected) {
    switch (expected) {
      case Token::Comma:       return Error::ExpectedComma;
      case Token::Colon:       return Error::ExpectedColon;
      case Token::ArrayBegin:  return Error::ExpectedArrayBegin;
      case Token::ArrayEnd:    return Error::ExpectedArrayEnd;
      case Token::ObjectBegin: return Error::ExpectedObjectBegin;
      case Token::ObjectEnd:   return Error::ExpectedObjectEnd;
      default:                 return Error::InvalidToken;
    }
  }

  _TRY(this->next_tok());
  return Ok{};
}

auto Deserializer::deserialize_null() -> Result<> {
  const auto tok = this->peek_tok();
  if (tok != Token::Null) {
    return Error::InvalidToken;
  }
  _TRY(this->next_tok());
  return Ok{};
}

auto Deserializer::deserialize_bool() -> Result<bool> {
  const auto tok = _TRY(this->peek_tok());
  if (tok != Token::True && tok != Token::False) {
    return Error::InvalidToken;
  }
  _TRY(this->next_tok());

  const auto val = tok == Token::True;
  return {val};
}

template <class T>
auto Deserializer::deserialize_int() -> Result<T> {
  const auto tok = _TRY(this->peek_tok());
  if (tok != Token::Number) {
    return Error::InvalidNumber;
  }

  const auto num_str = _TRY(this->next_tok());
  const auto num_res = num_str.parse<T>().ok_or(Error::InvalidNumber);
  return num_res;
}

template auto Deserializer::deserialize_int() -> Result<signed char>;
template auto Deserializer::deserialize_int() -> Result<short>;
template auto Deserializer::deserialize_int() -> Result<int>;
template auto Deserializer::deserialize_int() -> Result<long>;
template auto Deserializer::deserialize_int() -> Result<long long>;

template auto Deserializer::deserialize_int() -> Result<unsigned char>;
template auto Deserializer::deserialize_int() -> Result<unsigned short>;
template auto Deserializer::deserialize_int() -> Result<unsigned int>;
template auto Deserializer::deserialize_int() -> Result<unsigned long>;
template auto Deserializer::deserialize_int() -> Result<unsigned long long>;

template auto Deserializer::deserialize_flt() -> Result<float>;
template auto Deserializer::deserialize_flt() -> Result<double>;

template <class T>
auto Deserializer::deserialize_flt() -> Result<T> {
  const auto tok = _TRY(this->peek_tok());
  if (tok != Token::Number) {
    return Error::InvalidNumber;
  }

  const auto num_str = _TRY(this->next_tok());
  const auto num_res = num_str.parse<T>().ok_or(Error::InvalidNumber);
  return num_res;
}

auto Deserializer::deserialize_str() -> Result<Str> {
  const auto tok = _TRY(this->peek_tok());
  if (tok != Token::String) {
    return Error::InvalidString;
  }
  const auto str = _TRY(this->next_tok());
  if (str.len() < 2) {
    return Error::InvalidString;
  }

  return str[{1, str._len - 1}];
}

auto Deserializer::as_seq() -> DeserializeSeq {
  return DeserializeSeq{*this};
}

auto DeserializeSeq::next_entry() -> Result<> {
  if (_finished) {
    return Ok{};
  }

  // '['
  if (_count == 0) {
    _TRY(_des.read_tok(Token::ArrayBegin));
  }

  // ']'
  {
    const auto tok_end = _TRY(_des.peek_tok());
    if (tok_end == Token::ArrayEnd) {
      _TRY(_des.next_tok());
      _finished = true;
      return Ok{};
    }
  }

  // ','
  if (_count != 0) {
    _TRY(_des.read_tok(Token::Comma));
  }

  _count += 1;
  return Ok{};
}

auto Deserializer::as_map() -> DeserializeMap {
  return DeserializeMap{*this};
}

auto DeserializeMap::next_key() -> Result<Str> {
  if (_finished) {
    return Str{};
  }

  // '{'
  if (_count == 0) {
    _TRY(_des.read_tok(Token::ObjectBegin));
  }

  // '}'
  {
    const auto tok_end = _TRY(_des.peek_tok());
    if (tok_end == Token::ObjectEnd) {
      _TRY(_des.next_tok());
      _finished = true;
      return Str{};
    }
  }

  // ','
  if (_count != 0) {
    _TRY(_des.read_tok(Token::Comma));
  }

  const auto key = _TRY(_des.deserialize_str());
  _TRY(_des.read_tok(Token::Colon));

  _count += 1;
  return key;
}

auto Deserializer::as_obj() -> DeserializeObj {
  return DeserializeObj{*this};
}

auto DeserializeObj::next_key() -> Result<Str> {
  if (_finished) {
    return Str{};
  }

  // '{'
  if (_count == 0) {
    _TRY(_des.read_tok(Token::ObjectBegin));
  }

  // '}'
  {
    const auto tok_end = _TRY(_des.peek_tok());
    if (tok_end == Token::ObjectEnd) {
      _TRY(_des.next_tok());
      _finished = true;
      return Str{};
    }
  }

  // ','
  if (_count != 0) {
    _TRY(_des.read_tok(Token::Comma));
  }

  const auto key = _TRY(_des.deserialize_str());
  _TRY(_des.read_tok(Token::Colon));

  _count += 1;
  return key;
}

}  // namespace sfc::serde::json
