#pragma once

#include "sfc/alloc.h"

namespace sfc::serde::xml {

template <class W>
class Serializer {
  W& _write;

 public:
  explicit Serializer(W& write) noexcept : _write{write} {}
  ~Serializer() noexcept = default;
  Serializer(const Serializer&) noexcept = delete;

 public:
  void serialize_null() {}

  void serialize_bool(const bool& val) {
    _write.write_str(val ? Str{"true"} : Str{"false"});
  }

  void serialize_char(const char& val) {
    _write.write_str({&val, 1});
  }

  void serialize_int(const auto& val) {
    auto f = fmt::Fmter{_write};
    f.write_val(val);
  }

  void serialize_flt(const auto& val) {
    auto f = fmt::Fmter{_write};
    f.write_val(val);
  }

  void serialize_str(Str val) {
    _write.write_str(val);
  }

  class Node;
  auto serialize_node(Str name) -> Node {
    return Node{*this, name};
  }

 private:
  void indent(u32 depth) {
    for (auto i = 0U; i < depth; ++i) {
      _write.write_str("    ");
    }
  }
};

template <class W>
class Serializer<W>::Node {
  Serializer& _inn;
  Str _name;
  u32 _depth = 0;
  u32 _cnt = 0;

 public:
  explicit Node(Serializer& ser, Str name, u32 depth = 0) noexcept : _inn{ser}, _name{name}, _depth{depth} {
    _inn.indent(_depth);
    _inn._write.write_str("<");
    _inn._write.write_str(name);
  }

  ~Node() noexcept {
    if (_cnt == 0) {
      _inn._write.write_str("/>\n");
      return;
    }
    _inn.indent(_depth);
    _inn._write.write_str("</");
    _inn._write.write_str(_name);
    _inn._write.write_str(">\n");
  }

  Node(const Node&) noexcept = delete;

  void attr(Str name, const auto& value) {
    auto f = fmt::Fmter{_inn._write};
    f.write_str(" ");
    f.write_str(name);
    f.write_str("=\"");
    f.write_val(value);
    f.write_str("\"");
  }

  auto serialize_node(Str name) -> Node {
    if (_cnt++ == 0) {
      _inn._write.write_str(">\n");
    }
    return Node{_inn, name, _depth + 1};
  }
};

}  // namespace sfc::serde::xml
