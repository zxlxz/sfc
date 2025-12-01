#include "sfc/serde/json.h"

namespace sfc::serde::json {

template <class W>
class Serializer {
  template <class T>
  using Result = result::Result<T, Error>;

  W& _write;

 public:
  explicit Serializer(W& write) noexcept : _write{write} {}
  ~Serializer() noexcept = default;
  Serializer(const Serializer&) noexcept = delete;

 public:
  void serialize_null() {
    const auto s = "null";
    _write.write_str(s);
  }

  void serialize_bool(bool val) {
    const auto s = val ? Str{"true"} : Str{"false"};
    return this->write_str(s);
  }

  void serialize_char(char val) {
    const char s[] = {'"', val, '"'};
    _write.write_str({s, 3});
  }

  void serialize_int(trait::int_ auto val) {
    char buf[32] = {};
    const auto s = fmt::Debug::to_str(buf, val);
    this->write_str(s);
  }

  void serialize_flt(trait::flt_ auto val) {
    char buf[32] = {};
    const auto s = fmt::Debug::to_str(buf, val);
    this->write_str(s);
  }

  void serialize_str(Str val) {
    this->write_str("\"");
    this->write_str(val);
    this->write_str("\"");
  }

  void serialize_bin(Slice<const u8> val) {
    const auto s = base64::encode(val);
    this->serialize_str(s);
  }

  struct SerSeq;
  auto serialize_seq() -> SerSeq {
    this->write_str("[");
    return SerSeq{*this};
  }

  struct SerMap;
  auto serialize_map() -> SerMap {
    return SerMap{this};
  }

 private:
  void write_str(Str s) {
    _write.write_str(s).unwrap();
  }
};



}
